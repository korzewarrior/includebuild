/* IncludeBuild

   #define BUILD_IMPLEMENTATION
   #include "build.h"

   int main(int argc, char** argv) {
       build_init(argc, argv);
       executable("app", "**.c");
       return build();
   }

   cc -o build build.c
   ./build

   C99; GCC, Clang, or MinGW; Linux, macOS, or Windows; CC0.
   https://includebuild.com
*/

#ifndef INCLUDE_BUILD_H
#define INCLUDE_BUILD_H

#if defined(BUILD_IMPLEMENTATION) && defined(__linux__) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Target Target;

/* Set after build_init and before build. String values must remain valid until
   build returns. Command-line options override the corresponding fields. */
typedef struct BuildConfig {
    const char* root;           /* project root; default "."                  */
    const char* output_dir;     /* artifact directory; default: root          */
    const char* state_dir;      /* incremental state; default: ".build"       */
    const char* cc;             /* C compiler;   default $CC,  cc  / gcc      */
    const char* cxx;            /* C++ compiler; default $CXX, c++ / g++      */
    const char* ar;             /* archiver;     default $AR,  ar             */
    const char* compile_flags;  /* flags for every compile, all modes         */
    const char* debug_flags;    /* default "-g -O0 -DDEBUG"                   */
    const char* release_flags;  /* default "-O2 -DNDEBUG"                     */
    const char* link_flags;     /* appended to every executable/shared link   */
    int jobs;                   /* parallel jobs; 0 = CPU count               */
    bool verbose;               /* print commands and rebuild reasons         */
    bool quiet;                 /* errors only                                */
    int color;                  /* -1 auto (default), 0 off, 1 on             */
    bool self_rebuild;          /* rebuild ./build when build.c changes       */
    bool release;               /* read-only: release mode was requested      */
} BuildConfig;

extern BuildConfig config;

/* Parse argv. Call before declaring targets. */
#define build_init(argc, argv) build_init_from((argc), (argv), __FILE__)
void build_init_from(int argc, char** argv, const char* script_source);

/* Names must be portable file names. */
Target* executable(const char* name, const char* pattern);
Target* static_library(const char* name, const char* pattern);
Target* shared_library(const char* name, const char* pattern);

/* Add a path or glob. `*` and `?` stop at directory boundaries; `**` does not.
   "**.c" matches every C file. Prefix the pattern with a directory to limit
   the search. Unmatched patterns are errors. */
void sources(Target* target, const char* pattern);

/* Add an include directory. Users inherit it. */
void include_dir(Target* target, const char* dir);

/* Append target-specific flags. */
void compile_flags(Target* target, const char* flags);
void link_flags(Target* target, const char* flags);

/* Add a library dependency. */
void use(Target* target, Target* library);

/* Run the selected command and return its exit status. */
int build(void);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_BUILD_H */

/* ========================================================================= */
/*                              IMPLEMENTATION                               */
/* ========================================================================= */

#if defined(BUILD_IMPLEMENTATION) && !defined(INCLUDE_BUILD_IMPLEMENTED)
#define INCLUDE_BUILD_IMPLEMENTED

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <direct.h>
  #include <io.h>
  #define INCB_SEP '\\'
#else
  #include <dirent.h>
  #include <fcntl.h>
  #include <sys/wait.h>
  #include <unistd.h>
  #ifdef __APPLE__
    #include <mach-o/dyld.h>
  #endif
  #define INCB_SEP '/'
#endif

/* ------------------------------------------------------------------ types */

typedef struct { char* data; size_t len, cap; } incb_buf;
typedef struct { char** items; size_t count, cap; } incb_strv;

typedef enum { INCB_EXE, INCB_STATIC, INCB_SHARED } incb_kind;
typedef enum { INCB_BUILD, INCB_CLEAN, INCB_RUN, INCB_COMPDB, INCB_HELP } incb_verb;

typedef struct {              /* one planned compile */
    Target* t;
    char* src_abs; char* src_arg;
    char* obj_abs; char* obj_arg;
    char* dep_abs;
    char* cmdf_abs;           /* remembered command line */
    char* cmd;
    bool cpp;
    bool need; const char* why;
} incb_act;

struct Target {
    char* name;
    incb_kind kind;
    incb_strv patterns;
    incb_strv sources;         /* absolute, sorted, unique */
    incb_strv includes;        /* absolute after freeze */
    incb_buf cflags, ldflags;
    Target** uses; size_t use_count, use_cap;
    /* computed during planning */
    Target** deps; size_t dep_count; /* transitive, in link order */
    size_t order_index;
    char* out_abs; char* out_arg;
    char* link_cmd; char* cmdf_abs;
    size_t act_lo, act_hi;    /* compile action range */
    bool needs_pic, uses_cpp;
    int mark;                 /* graph-walk state */
    bool wanted;              /* part of the requested subgraph */
    bool link_need; const char* link_why;
};

BuildConfig config; /* zeroed; defaults are filled in by build_init */

static struct {
    bool inited, built;
    int argc; char** argv;
    const char* script_src;
    char* script_abs;
    /* command line */
    incb_verb verb;
    bool release;
    const char* run_target;
    char** run_args; int run_argc;
    int cli_jobs, cli_verbose, cli_quiet, cli_color; /* -1 = not given */
    /* effective settings */
    int jobs; bool verbose, quiet, color;
    /* frozen configuration (absolute paths) */
    char* root; char* out_dir; char* state_dir;
    /* build graph */
    Target** targets; size_t tcount, tcap;
    Target** order;        /* topological, dependencies first */
    incb_act* acts; size_t acount, acap;
} incb_g;

/* ---------------------------------------------------------------- reports */

#define INCB_CRED  (incb_g.color ? "\x1b[31m" : "")
#define INCB_CGRN  (incb_g.color ? "\x1b[32m" : "")
#define INCB_CDIM  (incb_g.color ? "\x1b[2m"  : "")
#define INCB_COFF  (incb_g.color ? "\x1b[0m"  : "")

#if defined(__GNUC__) || defined(__clang__)
#define INCB_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define INCB_NORETURN __declspec(noreturn)
#else
#define INCB_NORETURN
#endif

static INCB_NORETURN void incb_fatal(const char* fmt, ...) {
    va_list ap;
    fprintf(stderr, "%sbuild:%s ", INCB_CRED, INCB_COFF);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(1);
}

static void incb_say(const char* fmt, ...) {
    va_list ap;
    if (incb_g.quiet) return;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fputc('\n', stdout);
    fflush(stdout);
}

static bool incb_tty(FILE* f) {
#ifdef _WIN32
    return _isatty(_fileno(f)) != 0;
#else
    return isatty(fileno(f)) != 0;
#endif
}

static void incb_color_on(void) {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD m = 0;
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &m)) {
        SetConsoleMode(h, m | 0x0004 /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */);
    }
#endif
}

/* ----------------------------------------------------------------- memory */
/* Arena freed at exit. */

typedef struct incb_blk { struct incb_blk* next; size_t used, cap; } incb_blk;
static incb_blk* incb_mem;

static void incb_mem_release(void) {
    while (incb_mem) {
        incb_blk* next = incb_mem->next;
        free(incb_mem);
        incb_mem = next;
    }
}

static void* incb_alloc(size_t n) {
    incb_blk* b = incb_mem;
    void* p;
    n = (n + 7u) & ~(size_t)7u;
    if (!b || b->cap - b->used < n) {
        size_t cap = 64 * 1024;
        if (cap < n) cap = n;
        b = (incb_blk*)malloc(sizeof(*b) + cap);
        if (!b) incb_fatal("out of memory");
        b->next = incb_mem; b->used = 0; b->cap = cap;
        incb_mem = b;
    }
    p = (char*)(b + 1) + b->used;
    b->used += n;
    return p;
}

static char* incb_strdup(const char* s) {
    size_t n;
    char* p;
    if (!s) s = "";
    n = strlen(s) + 1;
    p = (char*)incb_alloc(n);
    memcpy(p, s, n);
    return p;
}

static char* incb_strndup(const char* s, size_t n) {
    char* p = (char*)incb_alloc(n + 1);
    memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

static char* incb_fmt(const char* fmt, ...) {
    va_list ap, ap2;
    int n;
    char* p;
    va_start(ap, fmt);
    va_copy(ap2, ap);
    n = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);
    if (n < 0) incb_fatal("vsnprintf failed");
    p = (char*)incb_alloc((size_t)n + 1);
    vsnprintf(p, (size_t)n + 1, fmt, ap);
    va_end(ap);
    return p;
}

static void incb_buf_grow(incb_buf* b, size_t extra) {
    size_t need = b->len + extra + 1;
    size_t cap;
    char* p;
    if (need <= b->cap) return;
    cap = b->cap ? b->cap : 128;
    while (cap < need) cap *= 2;
    p = (char*)incb_alloc(cap);
    if (b->len) memcpy(p, b->data, b->len);
    b->data = p;
    b->cap = cap;
}

static void incb_addn(incb_buf* b, const char* s, size_t n) {
    incb_buf_grow(b, n);
    if (n) memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}

static void incb_add(incb_buf* b, const char* s)  { incb_addn(b, s ? s : "", s ? strlen(s) : 0); }
static void incb_addc(incb_buf* b, char c)        { incb_addn(b, &c, 1); }

static char* incb_take(incb_buf* b) {
    char* s;
    incb_buf_grow(b, 0);
    b->data[b->len] = '\0';
    s = b->data;
    b->data = NULL; b->len = b->cap = 0;
    return s;
}

/* space-separated accumulation (per-target flag strings) */
static void incb_add_words(incb_buf* b, const char* s) {
    if (!s || !s[0]) return;
    if (b->len) incb_addc(b, ' ');
    incb_add(b, s);
}

static void incb_strv_push(incb_strv* v, char* s) {
    if (v->count == v->cap) {
        size_t cap = v->cap ? v->cap * 2 : 8;
        char** items = (char**)incb_alloc(cap * sizeof(*items));
        if (v->count) memcpy(items, v->items, v->count * sizeof(*items));
        v->items = items;
        v->cap = cap;
    }
    v->items[v->count++] = s;
}

static bool incb_strv_has(const incb_strv* v, const char* s) {
    size_t i;
    for (i = 0; i < v->count; ++i)
        if (strcmp(v->items[i], s) == 0) return true;
    return false;
}

static int incb_cmp_str(const void* a, const void* b) {
    return strcmp(*(char* const*)a, *(char* const*)b);
}

static void incb_strv_sort_unique(incb_strv* v) {
    size_t i, out = 0;
    if (v->count < 2) return;
    qsort(v->items, v->count, sizeof(*v->items), incb_cmp_str);
    for (i = 0; i < v->count; ++i) {
        if (out == 0 || strcmp(v->items[i], v->items[out - 1]) != 0)
            v->items[out++] = v->items[i];
    }
    v->count = out;
}

/* ------------------------------------------------------------------ paths */

static bool incb_is_sep(char c) { return c == '/' || c == '\\'; }

static bool incb_is_abs(const char* p) {
    if (!p || !p[0]) return false;
    if (incb_is_sep(p[0])) return true;
    return isalpha((unsigned char)p[0]) && p[1] == ':' && incb_is_sep(p[2]);
}

static char* incb_cwd(void) {
#ifdef _WIN32
    DWORD n = GetCurrentDirectoryA(0, NULL);
    char* buf = (char*)incb_alloc((size_t)n + 1);
    if (!n || GetCurrentDirectoryA(n + 1, buf) == 0)
        incb_fatal("cannot read current directory");
    return buf;
#else
    size_t size = 256;
    for (;;) {
        char* buf = (char*)incb_alloc(size);
        errno = 0;
        if (getcwd(buf, size)) return buf;
        if (errno != ERANGE) incb_fatal("cannot read current directory: %s", strerror(errno));
        size *= 2;
    }
#endif
}

/* Resolve `in` against absolute `base`, collapse . and .., use INCB_SEP. */
static char* incb_norm(const char* base, const char* in) {
    incb_buf out = {NULL, 0, 0};
    incb_strv segs = {NULL, 0, 0};
    size_t i;
    char root[4] = {0, 0, 0, 0};
    const char* parts[2];
    int pi;

    if (!in || !in[0]) incb_fatal("expected a non-empty path");
    parts[0] = incb_is_abs(in) ? NULL : base;
    parts[1] = in;
    {
        const char* first = parts[0] ? parts[0] : parts[1];
        if (isalpha((unsigned char)first[0]) && first[1] == ':' && incb_is_sep(first[2])) {
            root[0] = first[0]; root[1] = ':'; root[2] = INCB_SEP;
        } else if (incb_is_sep(first[0])) {
            root[0] = INCB_SEP;
        } else {
            incb_fatal("cannot resolve relative path '%s' without a base", in);
        }
    }
    for (pi = 0; pi < 2; ++pi) {
        const char* p = parts[pi];
        if (!p) continue;
        if (isalpha((unsigned char)p[0]) && p[1] == ':') p += 2;
        while (*p) {
            const char* start;
            size_t len;
            while (*p && incb_is_sep(*p)) ++p;
            if (!*p) break;
            start = p;
            while (*p && !incb_is_sep(*p)) ++p;
            len = (size_t)(p - start);
            if (len == 1 && start[0] == '.') continue;
            if (len == 2 && start[0] == '.' && start[1] == '.') {
                if (segs.count) --segs.count;
                continue;
            }
            incb_strv_push(&segs, incb_strndup(start, len));
        }
    }
    incb_add(&out, root);
    for (i = 0; i < segs.count; ++i) {
        if (i) incb_addc(&out, INCB_SEP);
        incb_add(&out, segs.items[i]);
    }
    return incb_take(&out);
}

static char* incb_join(const char* a, const char* b) {
    incb_buf out = {NULL, 0, 0};
    if (!a || !a[0]) return incb_strdup(b);
    if (!b || !b[0]) return incb_strdup(a);
    incb_add(&out, a);
    if (!incb_is_sep(a[strlen(a) - 1])) incb_addc(&out, INCB_SEP);
    while (*b && incb_is_sep(*b)) ++b;
    incb_add(&out, b);
    return incb_take(&out);
}

static char* incb_dirname(const char* path) {
    const char* last = strrchr(path, INCB_SEP);
    char* out;
    size_t n;
    if (!last) return incb_strdup(".");
    if (last == path) return incb_strdup("/");
    n = (size_t)(last - path);
    out = (char*)incb_alloc(n + 1);
    memcpy(out, path, n);
    out[n] = '\0';
    return out;
}

static const char* incb_basename(const char* path) {
    const char* a = strrchr(path, '/');
    const char* b = strrchr(path, '\\');
    const char* last = !a ? b : (!b ? a : (a > b ? a : b));
    return last ? last + 1 : path;
}

static bool incb_ceq(char a, char b) {
#ifdef _WIN32
    return tolower((unsigned char)a) == tolower((unsigned char)b);
#else
    return a == b;
#endif
}

static bool incb_path_eq(const char* a, const char* b) {
    size_t i;
    for (i = 0; a[i] && b[i]; ++i)
        if (!incb_ceq(a[i], b[i])) return false;
    return a[i] == b[i];
}

static bool incb_within(const char* root, const char* path) {
    size_t n = strlen(root), i;
    if (!n || strlen(path) < n) return false;
    for (i = 0; i < n; ++i)
        if (!incb_ceq(root[i], path[i])) return false;
    if (path[n] == '\0') return true;
    return incb_is_sep(root[n - 1]) || incb_is_sep(path[n]);
}

/* Path as written on command lines: root-relative when inside the root, and
   always forward-slashed — GCC-style drivers accept that on every platform,
   and it keeps backslashes (the escape character) out of depfiles. */
static char* incb_arg(const char* abs) {
    char* r;
    if (incb_within(incb_g.root, abs)) {
        const char* p = abs + strlen(incb_g.root);
        while (*p && incb_is_sep(*p)) ++p;
        r = incb_strdup(*p ? p : ".");
    } else {
        r = incb_strdup(abs);
    }
#ifdef _WIN32
    {
        char* p;
        for (p = r; *p; ++p) if (*p == '\\') *p = '/';
    }
#endif
    return r;
}

/* -------------------------------------------------------- stat cache & fs */

static unsigned long long incb_hash(const char* s) {
    unsigned long long h = 1469598103934665603ULL;
    while (*s) { h ^= (unsigned char)*s++; h *= 1099511628211ULL; }
    return h;
}

/* mtime in ns, 0 = missing */
static unsigned long long incb_stat_fs(const char* path) {
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA data;
    ULARGE_INTEGER ticks;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &data)) return 0;
    ticks.LowPart = data.ftLastWriteTime.dwLowDateTime;
    ticks.HighPart = data.ftLastWriteTime.dwHighDateTime;
    return ticks.QuadPart * 100ULL;
#else
    struct stat st;
    if (stat(path, &st) != 0) return 0;
#ifdef __APPLE__
    return (unsigned long long)st.st_mtimespec.tv_sec * 1000000000ULL +
           (unsigned long long)st.st_mtimespec.tv_nsec;
#else
    return (unsigned long long)st.st_mtim.tv_sec * 1000000000ULL +
           (unsigned long long)st.st_mtim.tv_nsec;
#endif
#endif
}

typedef struct { const char* path; unsigned long long mtime; } incb_stent;
static struct { incb_stent* e; size_t cap, n; } incb_stats;

static incb_stent* incb_stat_slot(const char* path) {
    size_t i;
    if (incb_stats.n * 4 >= incb_stats.cap * 3) {
        size_t cap = incb_stats.cap ? incb_stats.cap * 2 : 1024;
        incb_stent* e = (incb_stent*)incb_alloc(cap * sizeof(*e));
        size_t j;
        memset(e, 0, cap * sizeof(*e));
        for (j = 0; j < incb_stats.cap; ++j) {
            if (!incb_stats.e[j].path) continue;
            i = (size_t)(incb_hash(incb_stats.e[j].path) & (cap - 1));
            while (e[i].path) i = (i + 1) & (cap - 1);
            e[i] = incb_stats.e[j];
        }
        incb_stats.e = e;
        incb_stats.cap = cap;
    }
    i = (size_t)(incb_hash(path) & (incb_stats.cap - 1));
    while (incb_stats.e[i].path && strcmp(incb_stats.e[i].path, path) != 0)
        i = (i + 1) & (incb_stats.cap - 1);
    return &incb_stats.e[i];
}

/* cached mtime — refresh with incb_stat_refresh after writing the file */
static unsigned long long incb_mtime(const char* path) {
    incb_stent* s = incb_stat_slot(path);
    if (!s->path) {
        s->path = incb_strdup(path);
        s->mtime = incb_stat_fs(path);
        incb_stats.n++;
    }
    return s->mtime;
}

static void incb_stat_refresh(const char* path) {
    incb_stent* s = incb_stat_slot(path);
    if (!s->path) { s->path = incb_strdup(path); incb_stats.n++; }
    s->mtime = incb_stat_fs(path);
}

static bool incb_is_dir_now(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return false;
#ifdef _WIN32
    return (st.st_mode & _S_IFDIR) != 0;
#else
    return S_ISDIR(st.st_mode);
#endif
}

/* Directory traversal must not follow symlinks. Windows checks reparse-point
   attributes directly while enumerating. */
#ifndef _WIN32
static bool incb_is_real_dir(const char* path) {
    struct stat st;
    return lstat(path, &st) == 0 && S_ISDIR(st.st_mode);
}
#endif

static void incb_mkdir_one(const char* path) {
#ifdef _WIN32
    if (_mkdir(path) != 0 && errno != EEXIST)
#else
    if (mkdir(path, 0755) != 0 && errno != EEXIST)
#endif
        incb_fatal("cannot create directory '%s': %s", path, strerror(errno));
}

static void incb_mkdirs(const char* abs) {
    char* work = incb_strdup(abs);
    char* p = work;
    if (incb_is_dir_now(work)) return;
    if (isalpha((unsigned char)p[0]) && p[1] == ':') p += 2;
    for (; *p; ++p) {
        if (*p == INCB_SEP && p != work && !incb_is_sep(p[-1])) {
            *p = '\0';
            if (work[0] && !incb_is_dir_now(work)) incb_mkdir_one(work);
            *p = INCB_SEP;
        }
    }
    if (!incb_is_dir_now(work)) incb_mkdir_one(work);
}

static void incb_rmrf(const char* path) {
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    if (attrs == INVALID_FILE_ATTRIBUTES) return;
    if ((attrs & FILE_ATTRIBUTE_DIRECTORY) && !(attrs & FILE_ATTRIBUTE_REPARSE_POINT)) {
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA(incb_join(path, "*"), &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
                incb_rmrf(incb_join(path, fd.cFileName));
            } while (FindNextFileA(h, &fd));
            FindClose(h);
        }
        if (_rmdir(path) != 0 && errno != ENOENT)
            incb_fatal("cannot remove directory '%s': %s", path, strerror(errno));
    } else if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
        if (_rmdir(path) != 0 && errno != ENOENT)
            incb_fatal("cannot remove directory link '%s': %s", path, strerror(errno));
    } else {
        if (remove(path) != 0 && errno != ENOENT)
            incb_fatal("cannot remove '%s': %s", path, strerror(errno));
    }
#else
    struct stat st;
    if (lstat(path, &st) != 0) return;
    if (S_ISDIR(st.st_mode)) {
        DIR* d = opendir(path);
        struct dirent* ent;
        if (!d) incb_fatal("cannot open directory '%s': %s", path, strerror(errno));
        while ((ent = readdir(d)) != NULL) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
            incb_rmrf(incb_join(path, ent->d_name));
        }
        closedir(d);
        if (rmdir(path) != 0 && errno != ENOENT)
            incb_fatal("cannot remove directory '%s': %s", path, strerror(errno));
    } else {
        if (remove(path) != 0 && errno != ENOENT)
            incb_fatal("cannot remove '%s': %s", path, strerror(errno));
    }
#endif
}

static char* incb_read_file(const char* path) {
    FILE* f = fopen(path, "rb");
    incb_buf out = {NULL, 0, 0};
    char chunk[4096];
    size_t n;
    if (!f) return NULL;
    for (;;) {
        n = fread(chunk, 1, sizeof(chunk), f);
        incb_addn(&out, chunk, n);
        if (n == sizeof(chunk)) continue;
        if (ferror(f)) {
            fclose(f);
            return NULL;
        }
        break;
    }
    fclose(f);
    return incb_take(&out);
}

static void incb_write_file(const char* path, const char* text) {
    FILE* f;
    incb_mkdirs(incb_dirname(path));
    f = fopen(path, "wb");
    if (!f) incb_fatal("cannot write '%s': %s", path, strerror(errno));
    fputs(text, f);
    if (fclose(f) != 0) incb_fatal("cannot write '%s': %s", path, strerror(errno));
}

/* ---------------------------------------------------------------- quoting */

#ifdef _WIN32
/* Quote one argument by the CreateProcess/CRT argv rules. */
static char* incb_quote(const char* s) {
    incb_buf b = {NULL, 0, 0};
    const char* p = s;
    if (*s && !strpbrk(s, " \t\"")) return incb_strdup(s);
    incb_addc(&b, '"');
    for (;;) {
        size_t n = 0, i;
        while (*p == '\\') { ++n; ++p; }
        if (!*p) {
            for (i = 0; i < 2 * n; ++i) incb_addc(&b, '\\');
            break;
        }
        if (*p == '"') {
            for (i = 0; i < 2 * n + 1; ++i) incb_addc(&b, '\\');
            incb_addc(&b, '"');
        } else {
            for (i = 0; i < n; ++i) incb_addc(&b, '\\');
            incb_addc(&b, *p);
        }
        ++p;
    }
    incb_addc(&b, '"');
    return incb_take(&b);
}
#else
/* Quote one argument for /bin/sh. */
static char* incb_quote(const char* s) {
    incb_buf b = {NULL, 0, 0};
    const char* p;
    bool plain = s[0] != '\0';
    for (p = s; plain && *p; ++p) {
        if (!(isalnum((unsigned char)*p) || strchr("@%+=:,./-_", *p))) plain = false;
    }
    if (plain) return incb_strdup(s);
    incb_addc(&b, '\'');
    for (p = s; *p; ++p) {
        if (*p == '\'') incb_add(&b, "'\"'\"'");
        else incb_addc(&b, *p);
    }
    incb_addc(&b, '\'');
    return incb_take(&b);
}
#endif

/* ------------------------------------------------------------------- glob */

static bool incb_is_cpp(const char* path) {
    const char* e = strrchr(path, '.');
    return e && (!strcmp(e, ".cpp") || !strcmp(e, ".cc") || !strcmp(e, ".cxx"));
}

static bool incb_is_source(const char* path) {
    const char* e = strrchr(path, '.');
    return e && (!strcmp(e, ".c") || incb_is_cpp(path));
}

static bool incb_skip_dir(const char* name) {
    return name[0] == '.';
}

/* `*` any run, `?` one char — within a single path segment */
static bool incb_wild(const char* name, const char* pat) {
    if (*pat == '\0') return *name == '\0';
    if (*pat == '*') {
        for (;;) {
            if (incb_wild(name, pat + 1)) return true;
            if (!*name) return false;
            ++name;
        }
    }
    if (!*name) return false;
    if (*pat == '?' || incb_ceq(*pat, *name)) return incb_wild(name + 1, pat + 1);
    return false;
}

typedef struct { char** segs; size_t nsegs; incb_strv* out; } incb_globctx;

static void incb_glob_walk(incb_globctx* g, const char* dir, size_t i);

static void incb_glob_entry(incb_globctx* g, const char* dir, const char* name, bool is_dir, size_t i) {
    bool last = (i + 1 == g->nsegs);
    if (!incb_wild(name, g->segs[i])) return;
    if (last) {
        char* child;
        if (is_dir || !incb_is_source(name)) return;
        child = incb_join(dir, name);
        if (incb_g.script_abs && incb_path_eq(child, incb_g.script_abs)) return;
        incb_strv_push(g->out, child);
    } else if (is_dir) {
        incb_glob_walk(g, incb_join(dir, name), i + 1);
    }
}

static void incb_glob_walk(incb_globctx* g, const char* dir, size_t i) {
    bool star2 = strcmp(g->segs[i], "**") == 0;
    if (incb_within(incb_g.state_dir, dir)) return;
    if (!incb_path_eq(incb_g.out_dir, incb_g.root) && incb_within(incb_g.out_dir, dir)) return;
    if (star2 && i + 1 < g->nsegs) incb_glob_walk(g, dir, i + 1);
#ifdef _WIN32
    {
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA(incb_join(dir, "*"), &fd);
        if (h == INVALID_HANDLE_VALUE) return;
        do {
            bool is_dir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 &&
                          (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0;
            if (!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..")) continue;
            if (is_dir && incb_skip_dir(fd.cFileName)) continue;
            if (star2) {
                if (is_dir) incb_glob_walk(g, incb_join(dir, fd.cFileName), i);
            } else {
                incb_glob_entry(g, dir, fd.cFileName, is_dir, i);
            }
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
#else
    {
        DIR* d = opendir(dir);
        struct dirent* ent;
        if (!d) return;
        while ((ent = readdir(d)) != NULL) {
            char* child;
            bool is_dir;
            if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;
            child = incb_join(dir, ent->d_name);
            is_dir = incb_is_real_dir(child);
            if (is_dir && incb_skip_dir(ent->d_name)) continue;
            if (star2) {
                if (is_dir) incb_glob_walk(g, child, i);
            } else {
                incb_glob_entry(g, dir, ent->d_name, is_dir, i);
            }
        }
        closedir(d);
    }
#endif
}

/* Expand one pattern into t->sources as absolute paths. */
static void incb_expand_pattern(Target* t, const char* pattern) {
    if (!strpbrk(pattern, "*?")) {
        char* abs = incb_norm(incb_g.root, pattern);
        if (incb_is_dir_now(abs))
            incb_fatal("target '%s': '%s' is a directory — use a glob such as \"%s/**.c\"",
                      t->name, pattern, pattern);
        if (incb_stat_fs(abs) == 0)
            incb_fatal("target '%s': source file '%s' does not exist", t->name, pattern);
        if (!incb_is_source(abs))
            incb_fatal("target '%s': '%s' is not a C/C++ source file", t->name, pattern);
        if (!incb_strv_has(&t->sources, abs)) incb_strv_push(&t->sources, abs);
        return;
    }
    {
        char* pat = incb_strdup(pattern);
        incb_strv segs = {NULL, 0, 0};
        size_t before = t->sources.count, i;
        char* p = pat;
        incb_globctx g;
        for (i = 0; pat[i]; ++i) if (pat[i] == '\\') pat[i] = '/';
        if (incb_is_abs(pat))
            incb_fatal("target '%s': glob patterns must be relative to the root: '%s'",
                      t->name, pattern);
        while (*p) {
            char* start = p;
            while (*p && *p != '/') ++p;
            if (*p) *p++ = '\0';
            if (!*start || !strcmp(start, ".")) continue;
            if (start[0] == '*' && start[1] == '*' && start[2] != '\0') {
                incb_strv_push(&segs, incb_strdup("**"));  /* "**.c" → "**" + "*.c" */
                incb_strv_push(&segs, incb_fmt("*%s", start + 2));
            } else {
                incb_strv_push(&segs, start);
            }
        }
        if (!segs.count) incb_fatal("target '%s': empty pattern", t->name);
        g.segs = segs.items; g.nsegs = segs.count; g.out = &t->sources;
        incb_glob_walk(&g, incb_g.root, 0);
        if (t->sources.count == before)
            incb_fatal("target '%s': pattern '%s' matched no source files", t->name, pattern);
    }
}

/* -------------------------------------------------------------- processes */

typedef struct {
#ifdef _WIN32
    HANDLE h;
#else
    pid_t pid;
#endif
} incb_proc;

/* Start `cmd` with cwd = project root. If capture_path is non-NULL, stdout
   and stderr go to that file; otherwise they are inherited. */
static bool incb_spawn(const char* cmd, const char* capture_path, incb_proc* out) {
#ifdef _WIN32
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    HANDLE cap = INVALID_HANDLE_VALUE;
    char* mutable_cmd = incb_strdup(cmd);
    BOOL ok;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    if (capture_path) {
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa); sa.lpSecurityDescriptor = NULL; sa.bInheritHandle = TRUE;
        cap = CreateFileA(capture_path, GENERIC_WRITE, FILE_SHARE_READ, &sa,
                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (cap == INVALID_HANDLE_VALUE) return false;
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = cap;
        si.hStdError = cap;
    }
    ok = CreateProcessA(NULL, mutable_cmd, NULL, NULL, capture_path ? TRUE : FALSE,
                        0, NULL, incb_g.root ? incb_g.root : NULL, &si, &pi);
    if (cap != INVALID_HANDLE_VALUE) CloseHandle(cap);
    if (!ok) return false;
    CloseHandle(pi.hThread);
    out->h = pi.hProcess;
    return true;
#else
    pid_t pid = fork();
    if (pid < 0) return false;
    if (pid == 0) {
        if (incb_g.root && chdir(incb_g.root) != 0) _exit(127);
        if (capture_path) {
            int fd = open(capture_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) _exit(127);
            dup2(fd, 1);
            dup2(fd, 2);
            if (fd > 2) close(fd);
        }
        execl("/bin/sh", "sh", "-c", cmd, (char*)NULL);
        _exit(127);
    }
    out->pid = pid;
    return true;
#endif
}

static int incb_proc_wait(incb_proc* p) {
#ifdef _WIN32
    DWORD code = 1;
    WaitForSingleObject(p->h, INFINITE);
    GetExitCodeProcess(p->h, &code);
    CloseHandle(p->h);
    return (int)code;
#else
    int st = 0;
    if (waitpid(p->pid, &st, 0) < 0) return 127;
    if (WIFEXITED(st)) return WEXITSTATUS(st);
    if (WIFSIGNALED(st)) return 128 + WTERMSIG(st);
    return 1;
#endif
}

/* Wait for any busy slot to finish; returns its index, exit code in *code. */
static size_t incb_wait_any(incb_proc* procs, const bool* busy, size_t nslots, int* code) {
#ifdef _WIN32
    HANDLE handles[MAXIMUM_WAIT_OBJECTS];
    size_t map[MAXIMUM_WAIT_OBJECTS];
    DWORD n = 0, hit;
    size_t i;
    DWORD c = 1;
    for (i = 0; i < nslots && n < MAXIMUM_WAIT_OBJECTS; ++i) {
        if (busy[i]) { handles[n] = procs[i].h; map[n] = i; ++n; }
    }
    hit = WaitForMultipleObjects(n, handles, FALSE, INFINITE);
    if (hit >= n) incb_fatal("WaitForMultipleObjects failed");
    i = map[hit];
    GetExitCodeProcess(procs[i].h, &c);
    CloseHandle(procs[i].h);
    *code = (int)c;
    return i;
#else
    int st = 0;
    pid_t pid;
    size_t i;
    do {
        pid = waitpid(-1, &st, 0);
    } while (pid < 0 && errno == EINTR);
    if (pid < 0) incb_fatal("waitpid failed: %s", strerror(errno));
    for (i = 0; i < nslots; ++i) {
        if (busy[i] && procs[i].pid == pid) {
            if (WIFEXITED(st)) *code = WEXITSTATUS(st);
            else if (WIFSIGNALED(st)) *code = 128 + WTERMSIG(st);
            else *code = 1;
            return i;
        }
    }
    incb_fatal("waitpid returned an unknown child");
    return 0;
#endif
}

static int incb_ncpu(void) {
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (int)si.dwNumberOfProcessors;
#else
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return n > 0 ? (int)n : 1;
#endif
}

static double incb_now(void) {
#ifdef _WIN32
    return (double)GetTickCount64() / 1000.0;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
#endif
}

/* ----------------------------------------------------------- self-rebuild */

static char* incb_self_path(void) {
#ifdef _WIN32
    char buf[4096];
    DWORD n = GetModuleFileNameA(NULL, buf, sizeof(buf));
    if (n == 0 || n >= sizeof(buf)) return NULL;
    return incb_strdup(buf);
#elif defined(__linux__)
    char buf[4096];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n <= 0) return NULL;
    buf[n] = '\0';
    return incb_strdup(buf);
#elif defined(__APPLE__)
    char buf[4096];
    uint32_t n = sizeof(buf);
    if (_NSGetExecutablePath(buf, &n) != 0) return NULL;
    return incb_strdup(buf);
#else
    return incb_g.argv && incb_g.argv[0] ? incb_norm(incb_cwd(), incb_g.argv[0]) : NULL;
#endif
}

static void incb_setenv(const char* name, const char* value) {
#ifdef _WIN32
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif
}

static const char* incb_boot_cc(void) {
    const char* cc = config.cc && config.cc[0] ? config.cc : getenv("CC");
#ifdef _WIN32
    return cc && cc[0] ? cc : "gcc";
#else
    return cc && cc[0] ? cc : "cc";
#endif
}

static void incb_self_rebuild(void) {
    static const char* header_file = __FILE__;
    char* self;
    char* src;
    char* cmd;
    unsigned long long self_mt, src_mt, hdr_mt;
    incb_proc p;
    int i;

    if (!config.self_rebuild || getenv("BUILD_RESTARTED")) return;
    if (!incb_g.script_src) return;
    self = incb_self_path();
    if (!self) return;
    src = incb_is_abs(incb_g.script_src) ? incb_strdup(incb_g.script_src)
                                       : incb_norm(incb_cwd(), incb_g.script_src);
    self_mt = incb_stat_fs(self);
    src_mt = incb_stat_fs(src);
    if (!self_mt || !src_mt) return;
    hdr_mt = incb_stat_fs(incb_is_abs(header_file) ? header_file
                                                 : incb_norm(incb_cwd(), header_file));
    if (src_mt <= self_mt && hdr_mt <= self_mt) return;

    incb_say("%s[build] build script changed; rebuilding itself%s", INCB_CDIM, INCB_COFF);
    cmd = incb_fmt("%s -o %s %s", incb_quote(incb_boot_cc()), incb_quote(self), incb_quote(src));
#ifdef _WIN32
    {
        char* old = incb_fmt("%s.old", self);
        DeleteFileA(old);
        if (!MoveFileExA(self, old, MOVEFILE_REPLACE_EXISTING))
            incb_fatal("self-rebuild: cannot move the running executable aside");
        if (!incb_spawn(cmd, NULL, &p) || incb_proc_wait(&p) != 0) {
            MoveFileExA(old, self, MOVEFILE_REPLACE_EXISTING);
            incb_fatal("self-rebuild failed — fix build.c, then: %s", cmd);
        }
    }
#else
    if (!incb_spawn(cmd, NULL, &p) || incb_proc_wait(&p) != 0)
        incb_fatal("self-rebuild failed — fix build.c, then: %s", cmd);
#endif
    incb_setenv("BUILD_RESTARTED", "1");
#ifdef _WIN32
    {
        incb_buf b = {NULL, 0, 0};
        incb_proc child;
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        incb_add(&b, incb_quote(self));
        for (i = 1; i < incb_g.argc; ++i) {
            incb_addc(&b, ' ');
            incb_add(&b, incb_quote(incb_g.argv[i]));
        }
        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        if (!CreateProcessA(NULL, incb_take(&b), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
            incb_fatal("self-rebuild: cannot restart the build script");
        CloseHandle(pi.hThread);
        child.h = pi.hProcess;
        exit(incb_proc_wait(&child));
    }
#else
    {
        char** argv = (char**)incb_alloc(((size_t)incb_g.argc + 1) * sizeof(char*));
        argv[0] = self;
        for (i = 1; i < incb_g.argc; ++i) argv[i] = incb_g.argv[i];
        argv[incb_g.argc] = NULL;
        execv(self, argv);
        incb_fatal("self-rebuild: cannot restart the build script: %s", strerror(errno));
    }
#endif
}

/* --------------------------------------------------------- name validation */

static bool incb_name_reserved(const char* s, size_t n) {
    static const char* three[4] = {"CON", "PRN", "AUX", "NUL"};
    char up[5];
    size_t i;
    if (n != 3 && n != 4) return false;
    for (i = 0; i < n; ++i) up[i] = (char)toupper((unsigned char)s[i]);
    up[n] = '\0';
    if (n == 3) {
        for (i = 0; i < 4; ++i) if (!strcmp(up, three[i])) return true;
        return false;
    }
    return (!strncmp(up, "COM", 3) || !strncmp(up, "LPT", 3)) && up[3] >= '1' && up[3] <= '9';
}

static void incb_check_name(const char* name) {
    size_t len, base;
    const char* dot;
    const char* p;
    if (!name || !name[0]) incb_fatal("target name must not be empty");
    len = strlen(name);
    if (name[0] == '.' || name[len - 1] == '.' || isspace((unsigned char)name[len - 1]))
        incb_fatal("target name '%s' is not a portable file name", name);
    for (p = name; *p; ++p) {
        if ((unsigned char)*p < 32 || strchr("<>:\"/\\|?*", *p))
            incb_fatal("target name '%s' contains '%c', which is not portable", name, *p);
    }
    dot = strchr(name, '.');
    base = dot ? (size_t)(dot - name) : len;
    if (base == 0 || incb_name_reserved(name, base))
        incb_fatal("target name '%s' is reserved on Windows", name);
}

/* -------------------------------------------------------------- public API */

static void incb_need_init(const char* fn) {
    if (!incb_g.inited)
        incb_fatal("%s: call build_init(argc, argv) first", fn);
    if (incb_g.built)
        incb_fatal("%s: targets cannot change after build()", fn);
}

static const char* incb_usage =
    "usage: ./build [debug|release] [verb] [options]\n"
    "\n"
    "verbs:\n"
    "  build            compile and link all targets (default)\n"
    "  run [target]     build one executable, then run it (its args after --)\n"
    "  clean            remove build outputs and state\n"
    "  compdb           write compile_commands.json (auto-refreshed after)\n"
    "  help             show this text and the project's targets\n"
    "\n"
    "options:\n"
    "  -jN              parallel jobs (default: CPU count)\n"
    "  -v, --verbose    print commands and why each action runs\n"
    "  -q, --quiet      errors only\n"
    "  --color, --no-color\n";

static int incb_parse_jobs(const char* text) {
    char* end;
    long value;
    if (!text || !text[0]) incb_fatal("-j needs a positive number");
    errno = 0;
    value = strtol(text, &end, 10);
    if (errno || *end || value < 1 || value > INT_MAX)
        incb_fatal("-j needs a positive number");
    return (int)value;
}

void build_init_from(int argc, char** argv, const char* script_source) {
    int i;
    bool after_dashdash = false;
    if (incb_g.inited) incb_fatal("build_init called twice");
    incb_g.inited = true;
    atexit(incb_mem_release);
    incb_g.argc = argc;
    incb_g.argv = argv;
    incb_g.script_src = script_source;
    incb_g.verb = INCB_BUILD;
    incb_g.cli_jobs = -1;
    incb_g.cli_verbose = -1;
    incb_g.cli_quiet = -1;
    incb_g.cli_color = -1;

    /* defaults; override any of these between build_init and build */
    config.root = ".";
    config.output_dir = NULL;
    config.state_dir = ".build";
    config.cc = NULL;
    config.cxx = NULL;
    config.ar = NULL;
    config.compile_flags = "";
    config.debug_flags = "-g -O0 -DDEBUG";
    config.release_flags = "-O2 -DNDEBUG";
    config.link_flags = "";
    config.jobs = 0;
    config.verbose = 0;
    config.quiet = 0;
    config.color = -1;
    config.self_rebuild = true;
    config.release = false;

    incb_g.run_args = (char**)incb_alloc((size_t)(argc > 0 ? argc : 1) * sizeof(char*));
    for (i = 1; i < argc; ++i) {
        const char* a = argv[i];
        if (after_dashdash) {
            incb_g.run_args[incb_g.run_argc++] = argv[i];
            continue;
        }
        if (!strcmp(a, "--")) { after_dashdash = true; continue; }
        if (!strcmp(a, "debug"))   { incb_g.release = false; continue; }
        if (!strcmp(a, "release")) { incb_g.release = true; continue; }
        if (!strcmp(a, "build"))   { incb_g.verb = INCB_BUILD; continue; }
        if (!strcmp(a, "clean"))   { incb_g.verb = INCB_CLEAN; continue; }
        if (!strcmp(a, "run"))     { incb_g.verb = INCB_RUN; continue; }
        if (!strcmp(a, "compdb"))  { incb_g.verb = INCB_COMPDB; continue; }
        if (!strcmp(a, "help") || !strcmp(a, "-h") || !strcmp(a, "--help")) { incb_g.verb = INCB_HELP; continue; }
        if (!strncmp(a, "-j", 2)) {
            const char* n = a[2] ? a + 2 : (i + 1 < argc ? argv[++i] : NULL);
            incb_g.cli_jobs = incb_parse_jobs(n);
            continue;
        }
        if (!strcmp(a, "-v") || !strcmp(a, "--verbose")) { incb_g.cli_verbose = 1; continue; }
        if (!strcmp(a, "-q") || !strcmp(a, "--quiet"))   { incb_g.cli_quiet = 1; continue; }
        if (!strcmp(a, "--color"))    { incb_g.cli_color = 1; continue; }
        if (!strcmp(a, "--no-color")) { incb_g.cli_color = 0; continue; }
        if (a[0] == '-') {
            fputs(incb_usage, stderr);
            incb_fatal("unknown option '%s'", a);
        }
        if (incb_g.verb == INCB_RUN && !incb_g.run_target) { incb_g.run_target = a; continue; }
        fputs(incb_usage, stderr);
        incb_fatal("unknown argument '%s' (arguments for `run` go after '--'; "
                  "for custom options, filter argv before build_init)", a);
    }
    config.release = incb_g.release;
}

static Target* incb_target_new(const char* name, const char* pattern, incb_kind kind) {
    Target* t;
    size_t i;
    incb_need_init("target creation");
    incb_check_name(name);
    for (i = 0; i < incb_g.tcount; ++i)
        if (!strcmp(incb_g.targets[i]->name, name))
            incb_fatal("target '%s' already exists", name);
    t = (Target*)incb_alloc(sizeof(*t));
    memset(t, 0, sizeof(*t));
    t->name = incb_strdup(name);
    t->kind = kind;
    if (!pattern || !pattern[0])
        incb_fatal("target '%s': source pattern must not be empty", name);
    incb_strv_push(&t->patterns, incb_strdup(pattern));
    if (incb_g.tcount == incb_g.tcap) {
        size_t cap = incb_g.tcap ? incb_g.tcap * 2 : 8;
        Target** ts = (Target**)incb_alloc(cap * sizeof(*ts));
        if (incb_g.tcount) memcpy(ts, incb_g.targets, incb_g.tcount * sizeof(*ts));
        incb_g.targets = ts;
        incb_g.tcap = cap;
    }
    incb_g.targets[incb_g.tcount++] = t;
    return t;
}

Target* executable(const char* name, const char* pattern) {
    return incb_target_new(name, pattern, INCB_EXE);
}

Target* static_library(const char* name, const char* pattern) {
    return incb_target_new(name, pattern, INCB_STATIC);
}

Target* shared_library(const char* name, const char* pattern) {
    return incb_target_new(name, pattern, INCB_SHARED);
}

static void incb_check_target(Target* t, const char* fn) {
    incb_need_init(fn);
    if (!t) incb_fatal("%s: target is NULL", fn);
}

void sources(Target* t, const char* pattern) {
    incb_check_target(t, "sources");
    if (!pattern || !pattern[0]) incb_fatal("sources: empty pattern for target '%s'", t->name);
    incb_strv_push(&t->patterns, incb_strdup(pattern));
}

void include_dir(Target* t, const char* dir) {
    incb_check_target(t, "include_dir");
    if (!dir || !dir[0]) incb_fatal("include_dir: empty path for target '%s'", t->name);
    incb_strv_push(&t->includes, incb_strdup(dir));
}

void compile_flags(Target* t, const char* flags) {
    incb_check_target(t, "compile_flags");
    incb_add_words(&t->cflags, flags);
}

void link_flags(Target* t, const char* flags) {
    incb_check_target(t, "link_flags");
    if (t->kind == INCB_STATIC)
        incb_fatal("link_flags: static library '%s' is archived, not linked", t->name);
    incb_add_words(&t->ldflags, flags);
}

void use(Target* t, Target* lib) {
    size_t i;
    incb_check_target(t, "use");
    if (!lib) incb_fatal("use: library target is NULL");
    if (lib->kind == INCB_EXE)
        incb_fatal("use: '%s' is an executable; only libraries can be used", lib->name);
    if (t == lib) incb_fatal("use: target '%s' cannot use itself", t->name);
    for (i = 0; i < t->use_count; ++i)
        if (t->uses[i] == lib) return;
    if (t->use_count == t->use_cap) {
        size_t cap = t->use_cap ? t->use_cap * 2 : 4;
        Target** us = (Target**)incb_alloc(cap * sizeof(*us));
        if (t->use_count) memcpy(us, t->uses, t->use_count * sizeof(*us));
        t->uses = us;
        t->use_cap = cap;
    }
    t->uses[t->use_count++] = lib;
}

/* ---------------------------------------------------------------- planning */

static const char* incb_tool_cc(void) { return incb_boot_cc(); }

static const char* incb_tool_cxx(void) {
    const char* s = config.cxx && config.cxx[0] ? config.cxx : getenv("CXX");
#ifdef _WIN32
    return s && s[0] ? s : "g++";
#else
    return s && s[0] ? s : "c++";
#endif
}

static const char* incb_tool_ar(void) {
    const char* s = config.ar && config.ar[0] ? config.ar : getenv("AR");
    return s && s[0] ? s : "ar";
}

static char* incb_out_filename(const Target* t) {
    switch (t->kind) {
        case INCB_STATIC: return incb_fmt("lib%s.a", t->name);
        case INCB_SHARED:
#ifdef _WIN32
            return incb_fmt("%s.dll", t->name);
#elif defined(__APPLE__)
            return incb_fmt("lib%s.dylib", t->name);
#else
            return incb_fmt("lib%s.so", t->name);
#endif
        default:
#ifdef _WIN32
            return incb_fmt("%s.exe", t->name);
#else
            return incb_strdup(t->name);
#endif
    }
}

static void incb_topo_visit(Target* t, Target** order, size_t* n) {
    size_t i;
    if (t->mark == 2) return;
    if (t->mark == 1) incb_fatal("dependency cycle involving target '%s'", t->name);
    t->mark = 1;
    for (i = 0; i < t->use_count; ++i) incb_topo_visit(t->uses[i], order, n);
    t->mark = 2;
    order[(*n)++] = t;
}

static void incb_mark_dependencies(Target* t, bool* marked) {
    size_t i;
    for (i = 0; i < t->use_count; ++i) {
        Target* dep = t->uses[i];
        if (marked[dep->order_index]) continue;
        marked[dep->order_index] = true;
        incb_mark_dependencies(dep, marked);
    }
}

/* Store each transitive dependency once, with users before the libraries they
   use. This is the order required by single-pass static linkers. */
static void incb_plan_dependencies(void) {
    size_t i;
    for (i = 0; i < incb_g.tcount; ++i) {
        Target* t = incb_g.targets[i];
        bool* marked = (bool*)incb_alloc((incb_g.tcount ? incb_g.tcount : 1) * sizeof(*marked));
        size_t k;
        memset(marked, 0, incb_g.tcount * sizeof(*marked));
        t->deps = (Target**)incb_alloc((incb_g.tcount ? incb_g.tcount : 1) * sizeof(*t->deps));
        incb_mark_dependencies(t, marked);
        for (k = incb_g.tcount; k-- > 0;)
            if (marked[k]) t->deps[t->dep_count++] = incb_g.order[k];
    }
}

#ifndef _WIN32
static bool incb_uses_shared(const Target* t) {
    size_t i;
    for (i = 0; i < t->dep_count; ++i)
        if (t->deps[i]->kind == INCB_SHARED) return true;
    return false;
}
#endif

static void incb_act_push(incb_act a) {
    if (incb_g.acount == incb_g.acap) {
        size_t cap = incb_g.acap ? incb_g.acap * 2 : 64;
        incb_act* as = (incb_act*)incb_alloc(cap * sizeof(*as));
        if (incb_g.acount) memcpy(as, incb_g.acts, incb_g.acount * sizeof(*as));
        incb_g.acts = as;
        incb_g.acap = cap;
    }
    incb_g.acts[incb_g.acount++] = a;
}

static char* incb_obj_rel(const char* src_abs) {
    if (incb_within(incb_g.root, src_abs)) {
        const char* p = src_abs + strlen(incb_g.root);
        while (*p && incb_is_sep(*p)) ++p;
        return incb_fmt("%s.o", p);
    }
    return incb_fmt("external%c%016llx_%s.o", INCB_SEP, incb_hash(src_abs), incb_basename(src_abs));
}

/* Effective include dirs for t: its own, then its dependencies' (deduped). */
static void incb_collect_incs(Target* t, incb_strv* out) {
    size_t i, j;
    for (i = 0; i < t->includes.count; ++i)
        if (!incb_strv_has(out, t->includes.items[i]))
            incb_strv_push(out, t->includes.items[i]);
    for (i = 0; i < t->dep_count; ++i) {
        for (j = 0; j < t->deps[i]->includes.count; ++j)
            if (!incb_strv_has(out, t->deps[i]->includes.items[j]))
                incb_strv_push(out, t->deps[i]->includes.items[j]);
    }
}

static const char* incb_mode_name(void) {
    return incb_g.release ? "release" : "debug";
}

static void incb_resolve_layout(void) {
    char* cwd = incb_cwd();
    size_t i, j;
    incb_g.root = incb_norm(cwd, config.root && config.root[0] ? config.root : ".");
    if (!incb_is_dir_now(incb_g.root))
        incb_fatal("project root '%s' does not exist", config.root);
    if (incb_g.script_src)
        incb_g.script_abs = incb_norm(cwd, incb_g.script_src);
    incb_g.state_dir = incb_norm(incb_g.root, config.state_dir && config.state_dir[0] ? config.state_dir : ".build");
    incb_g.out_dir = config.output_dir && config.output_dir[0] ? incb_norm(incb_g.root, config.output_dir)
                                                : incb_strdup(incb_g.root);
    if (incb_path_eq(incb_g.state_dir, incb_g.root) || incb_within(incb_g.state_dir, incb_g.root))
        incb_fatal("state_dir must not be the project root or contain it");
    if (incb_path_eq(incb_g.out_dir, incb_g.state_dir) || incb_within(incb_g.state_dir, incb_g.out_dir))
        incb_fatal("out_dir must not be inside state_dir");

    /* outputs exist for every verb that needs them (build, clean, run) */
    for (i = 0; i < incb_g.tcount; ++i) {
        Target* t = incb_g.targets[i];
        t->out_abs = incb_join(incb_g.out_dir, incb_out_filename(t));
        t->out_arg = incb_arg(t->out_abs);
        for (j = 0; j < i; ++j)
            if (incb_path_eq(incb_g.targets[j]->out_abs, t->out_abs))
                incb_fatal("targets '%s' and '%s' produce the same output: %s",
                          incb_g.targets[j]->name, t->name, t->out_arg);
    }
}

static void incb_resolve_graph(void) {
    size_t i;
    incb_g.order = (Target**)incb_alloc((incb_g.tcount ? incb_g.tcount : 1) * sizeof(*incb_g.order));
    {
        size_t n = 0;
        for (i = 0; i < incb_g.tcount; ++i) incb_g.targets[i]->mark = 0;
        for (i = 0; i < incb_g.tcount; ++i) incb_topo_visit(incb_g.targets[i], incb_g.order, &n);
    }
    for (i = 0; i < incb_g.tcount; ++i) incb_g.order[i]->order_index = i;
    incb_plan_dependencies();
}

static void incb_resolve_inputs(void) {
    size_t i, j;
    for (i = 0; i < incb_g.tcount; ++i) {
        Target* t = incb_g.targets[i];
        for (j = 0; j < t->includes.count; ++j) {
            char* abs = incb_norm(incb_g.root, t->includes.items[j]);
            if (!incb_is_dir_now(abs))
                incb_fatal("target '%s': include directory '%s' does not exist",
                          t->name, t->includes.items[j]);
            t->includes.items[j] = abs;
        }
        for (j = 0; j < t->patterns.count; ++j)
            incb_expand_pattern(t, t->patterns.items[j]);
        incb_strv_sort_unique(&t->sources);
        if (!t->sources.count)
            incb_fatal("target '%s' has no sources — call sources()", t->name);
        for (j = 0; j < t->sources.count; ++j)
            if (incb_is_cpp(t->sources.items[j])) t->uses_cpp = true;
    }

    /* dependencies first in order[]: propagate uses_cpp up ... */
    for (i = 0; i < incb_g.tcount; ++i) {
        Target* t = incb_g.order[i];
        for (j = 0; j < t->use_count; ++j)
            if (t->uses[j]->uses_cpp) t->uses_cpp = true;
    }
    /* ... and needs_pic down (users come last in order[]) */
    for (i = incb_g.tcount; i-- > 0;) {
        Target* t = incb_g.order[i];
        if (t->kind == INCB_SHARED || t->needs_pic) {
            for (j = 0; j < t->use_count; ++j)
                if (t->uses[j]->kind == INCB_STATIC) t->uses[j]->needs_pic = true;
        }
    }
}

static void incb_plan_compile_actions(void) {
    const char* mode_flags = incb_g.release ? config.release_flags : config.debug_flags;
    size_t i, j;
    for (i = 0; i < incb_g.tcount; ++i) {
        Target* t = incb_g.targets[i];
        incb_strv incs = {NULL, 0, 0};
        incb_buf common = {NULL, 0, 0};
        char* objdir = incb_fmt("%s%cobj%c%s%c%s", incb_g.state_dir, INCB_SEP, INCB_SEP,
                               incb_mode_name(), INCB_SEP, t->name);
        incb_collect_incs(t, &incs);

        if (config.compile_flags && config.compile_flags[0])   { incb_addc(&common, ' '); incb_add(&common, config.compile_flags); }
        if (mode_flags && mode_flags[0]) { incb_addc(&common, ' '); incb_add(&common, mode_flags); }
        for (j = 0; j < incs.count; ++j) {
            incb_add(&common, " -I");
            incb_add(&common, incb_quote(incb_arg(incs.items[j])));
        }
#ifndef _WIN32
        if (t->kind == INCB_SHARED || t->needs_pic) incb_add(&common, " -fPIC");
#endif
        if (t->cflags.len) { incb_addc(&common, ' '); incb_add(&common, t->cflags.data); }

        t->act_lo = incb_g.acount;
        for (j = 0; j < t->sources.count; ++j) {
            incb_act a;
            memset(&a, 0, sizeof(a));
            a.t = t;
            a.cpp = incb_is_cpp(t->sources.items[j]);
            a.src_abs = t->sources.items[j];
            a.src_arg = incb_arg(a.src_abs);
            a.obj_abs = incb_join(objdir, incb_obj_rel(a.src_abs));
            a.obj_arg = incb_arg(a.obj_abs);
            a.dep_abs = incb_fmt("%s.d", a.obj_abs);
            a.cmdf_abs = incb_fmt("%s.cmd", a.obj_abs);
            a.cmd = incb_fmt("%s%s -MMD -MF %s -c %s -o %s",
                            incb_quote(a.cpp ? incb_tool_cxx() : incb_tool_cc()),
                            common.len ? common.data : "",
                            incb_quote(incb_fmt("%s.d", a.obj_arg)),
                            incb_quote(a.src_arg),
                            incb_quote(a.obj_arg));
            incb_act_push(a);
        }
        t->act_hi = incb_g.acount;
    }
}

static void incb_add_platform_link_options(incb_buf* b, const Target* t) {
#ifdef _WIN32
    if (t->kind == INCB_SHARED) incb_add(b, " -shared");
#elif defined(__APPLE__)
    if (t->kind == INCB_SHARED) {
        incb_add(b, " -dynamiclib ");
        incb_add(b, incb_quote(incb_fmt("-Wl,-install_name,@rpath/%s", incb_basename(t->out_arg))));
    }
    if (incb_uses_shared(t)) {
        incb_addc(b, ' ');
        incb_add(b, incb_quote("-Wl,-rpath,@loader_path"));
    }
#else
    if (t->kind == INCB_SHARED) {
        incb_add(b, " -shared ");
        incb_add(b, incb_quote(incb_fmt("-Wl,-soname,%s", incb_basename(t->out_arg))));
    }
    if (incb_uses_shared(t)) {
        incb_addc(b, ' ');
        incb_add(b, incb_quote("-Wl,-rpath,$ORIGIN"));
    }
#endif
}

static void incb_plan_link_actions(void) {
    size_t i;
    for (i = 0; i < incb_g.tcount; ++i) {
        Target* t = incb_g.targets[i];
        incb_buf b = {NULL, 0, 0};
        size_t k;
        if (t->kind == INCB_STATIC) {
            incb_add(&b, incb_quote(incb_tool_ar()));
            incb_add(&b, " rcs ");
            incb_add(&b, incb_quote(t->out_arg));
        } else {
            incb_add(&b, incb_quote(t->uses_cpp ? incb_tool_cxx() : incb_tool_cc()));
            incb_add_platform_link_options(&b, t);
            incb_add(&b, " -o ");
            incb_add(&b, incb_quote(t->out_arg));
        }
        for (k = t->act_lo; k < t->act_hi; ++k) {
            incb_addc(&b, ' ');
            incb_add(&b, incb_quote(incb_g.acts[k].obj_arg));
        }
        if (t->kind != INCB_STATIC) {
            for (k = 0; k < t->dep_count; ++k) {
                incb_addc(&b, ' ');
                incb_add(&b, incb_quote(t->deps[k]->out_arg));
            }
            if (t->ldflags.len) { incb_addc(&b, ' '); incb_add(&b, t->ldflags.data); }
            if (config.link_flags && config.link_flags[0]) { incb_addc(&b, ' '); incb_add(&b, config.link_flags); }
        }
        t->link_cmd = incb_take(&b);
        t->cmdf_abs = incb_fmt("%s%clink%c%s%c%s.cmd", incb_g.state_dir, INCB_SEP, INCB_SEP,
                              incb_mode_name(), INCB_SEP, t->name);
    }
}

/* ------------------------------------------------------------- incremental */

/* Check the object's Make-style depfile; false = must rebuild (*why says). */
static bool incb_deps_ok(incb_act* a, const char** why) {
    char* raw = incb_read_file(a->dep_abs);
    char* cur;
    char* colon = NULL;
    unsigned long long obj_mt = incb_mtime(a->obj_abs);
    if (!raw) { *why = "missing depfile"; return false; }
    for (cur = raw; *cur; ++cur) {                    /* splice continuations */
        if (cur[0] == '\\' && cur[1] == '\n') { cur[0] = ' '; cur[1] = ' '; }
        else if (cur[0] == '\\' && cur[1] == '\r' && cur[2] == '\n') {
            cur[0] = ' '; cur[1] = ' '; cur[2] = ' ';
        }
    }
    for (cur = raw; *cur; ++cur) {
        if (*cur == ':' && (cur[1] == '\0' || isspace((unsigned char)cur[1]))) { colon = cur; break; }
    }
    if (!colon) { *why = "unreadable depfile"; return false; }
    cur = colon + 1;
    while (*cur) {
        incb_buf tok = {NULL, 0, 0};
        while (*cur && isspace((unsigned char)*cur)) ++cur;
        if (!*cur) break;
        while (*cur && !isspace((unsigned char)*cur)) {
            if (*cur == '\\' && cur[1]) { incb_addc(&tok, cur[1]); cur += 2; }
            else incb_addc(&tok, *cur++);
        }
        if (tok.len) {
            char* dep = incb_norm(incb_g.root, tok.data);
            unsigned long long mt = incb_mtime(dep);
            if (mt == 0) { *why = incb_fmt("%s disappeared", incb_arg(dep)); return false; }
            if (mt > obj_mt) { *why = incb_fmt("%s changed", incb_arg(dep)); return false; }
        }
    }
    return true;
}

static void incb_decide_compile(incb_act* a) {
    char* prev;
    a->need = true;
    if (incb_mtime(a->obj_abs) == 0) { a->why = "no object yet"; return; }
    prev = incb_read_file(a->cmdf_abs);
    if (!prev) { a->why = "no record of the last build"; return; }
    if (strcmp(prev, a->cmd) != 0) { a->why = "compile command changed"; return; }
    if (!incb_deps_ok(a, &a->why)) return;
    a->need = false;
}

/* Decide in topological order so dependency decisions are already made. */
static void incb_decide_link(Target* t) {
    char* prev;
    size_t k;
    unsigned long long out_mt;
    t->link_need = true;
    for (k = 0; t->kind != INCB_STATIC && k < t->dep_count; ++k) {
        if (t->deps[k]->link_need) {
            t->link_why = incb_fmt("dependency '%s' will relink", t->deps[k]->name);
            return;
        }
    }
    out_mt = incb_mtime(t->out_abs);
    if (out_mt == 0) { t->link_why = "no output yet"; return; }
    prev = incb_read_file(t->cmdf_abs);
    if (!prev) { t->link_why = "no record of the last link"; return; }
    if (strcmp(prev, t->link_cmd) != 0) { t->link_why = "link command changed"; return; }
    for (k = t->act_lo; k < t->act_hi; ++k) {
        if (incb_g.acts[k].need) {
            t->link_why = "an object was rebuilt";
            return;
        }
        if (incb_mtime(incb_g.acts[k].obj_abs) > out_mt) {
            t->link_why = "objects newer than output";
            return;
        }
    }
    for (k = 0; t->kind != INCB_STATIC && k < t->dep_count; ++k) {
        if (incb_mtime(t->deps[k]->out_abs) > out_mt) {
            t->link_why = incb_fmt("dependency '%s' is newer", t->deps[k]->name);
            return;
        }
    }
    t->link_need = false;
}

/* --------------------------------------------------------------- execution */

static char* incb_capture_path(size_t slot) {
    return incb_fmt("%s%ctmp%cjob%u.log", incb_g.state_dir, INCB_SEP, INCB_SEP, (unsigned)slot);
}

static void incb_progress(size_t done, size_t total, const char* tool, const char* text) {
    incb_say("%s[%u/%u]%s %s %s", INCB_CDIM, (unsigned)done, (unsigned)total, INCB_COFF, tool, text);
}

static void incb_report(const char* label, const char* cmd, const char* capture, int code) {
    char* out = incb_read_file(capture);
    if (code != 0) {
        fprintf(stderr, "%sFAILED%s %s %s(exit %d)%s\n%s%s%s\n",
                INCB_CRED, INCB_COFF, label, INCB_CDIM, code, INCB_COFF,
                INCB_CDIM, cmd, INCB_COFF);
        if (out && out[0]) fputs(out, stderr);
    } else if (out && out[0] && !incb_g.quiet) {
        fputs(out, stdout);          /* warnings from a successful command */
    }
}

/* Run every needed compile action through the process pool. */
static size_t incb_compile_phase(size_t* compiled) {
    size_t total = 0, started = 0, failures = 0, next = 0;
    size_t nslots = (size_t)incb_g.jobs, s, i;
    bool* busy;
    incb_proc* procs;
    incb_act** payload;
    char** caps;
    bool draining = false;

    *compiled = 0;
    for (i = 0; i < incb_g.acount; ++i) {
        incb_act* a = &incb_g.acts[i];
        if (!a->t->wanted) { a->need = false; continue; }
        incb_decide_compile(a);
        if (a->need) ++total;
    }
    if (!total) return 0;

    busy = (bool*)incb_alloc(nslots * sizeof(*busy));
    procs = (incb_proc*)incb_alloc(nslots * sizeof(*procs));
    payload = (incb_act**)incb_alloc(nslots * sizeof(*payload));
    caps = (char**)incb_alloc(nslots * sizeof(*caps));
    memset(busy, 0, nslots * sizeof(*busy));
    for (s = 0; s < nslots; ++s) caps[s] = incb_capture_path(s);

    for (;;) {
        size_t running = 0;
        if (!draining) {
            for (s = 0; s < nslots; ++s) {
                incb_act* a;
                if (busy[s]) continue;
                while (next < incb_g.acount && !incb_g.acts[next].need) ++next;
                if (next >= incb_g.acount) break;
                a = &incb_g.acts[next++];
                incb_mkdirs(incb_dirname(a->obj_abs));
                ++started;
                incb_progress(started, total, a->cpp ? "c++" : "cc ",
                             incb_g.tcount > 1
                                 ? incb_fmt("%s %s(%s)%s", a->src_arg, INCB_CDIM, a->t->name, INCB_COFF)
                                 : a->src_arg);
                if (incb_g.verbose) {
                    incb_say("%s  $ %s%s", INCB_CDIM, a->cmd, INCB_COFF);
                    incb_say("%s  because: %s%s", INCB_CDIM, a->why, INCB_COFF);
                }
                if (!incb_spawn(a->cmd, caps[s], &procs[s]))
                    incb_fatal("cannot start '%s' — is it installed?",
                              a->cpp ? incb_tool_cxx() : incb_tool_cc());
                busy[s] = true;
                payload[s] = a;
            }
        }
        for (s = 0; s < nslots; ++s) if (busy[s]) ++running;
        if (!running) break;
        {
            int code = 0;
            size_t hit = incb_wait_any(procs, busy, nslots, &code);
            incb_act* a = payload[hit];
            busy[hit] = false;
            incb_report(incb_fmt("compiling %s (%s)", a->src_arg, a->t->name),
                       a->cmd, caps[hit], code);
            if (code == 0) {
                incb_write_file(a->cmdf_abs, a->cmd);
                incb_stat_refresh(a->obj_abs);
                ++*compiled;
            } else {
                ++failures;
                draining = true;
            }
        }
    }
    return failures;
}

/* Link every target that needs it, respecting use ordering. */
static size_t incb_link_phase(size_t* linked) {
    size_t i, s;
    size_t nslots = (size_t)incb_g.jobs;
    size_t total = 0, started = 0, remaining = 0, failures = 0;
    int* st;                     /* 0 waiting, 1 running, 2 settled */
    bool* busy;
    incb_proc* procs;
    Target** payload;
    char** caps;
    bool draining = false;

    *linked = 0;
    st = (int*)incb_alloc((incb_g.tcount ? incb_g.tcount : 1) * sizeof(int));
    for (i = 0; i < incb_g.tcount; ++i) {
        Target* t = incb_g.order[i];
        if (!t->wanted) { t->link_need = false; st[i] = 2; continue; }
        incb_decide_link(t);
        st[i] = t->link_need ? 0 : 2;
        if (t->link_need) { ++total; ++remaining; }
    }
    if (!total) return 0;

    busy = (bool*)incb_alloc(nslots * sizeof(*busy));
    procs = (incb_proc*)incb_alloc(nslots * sizeof(*procs));
    payload = (Target**)incb_alloc(nslots * sizeof(*payload));
    caps = (char**)incb_alloc(nslots * sizeof(*caps));
    memset(busy, 0, nslots * sizeof(*busy));
    for (s = 0; s < nslots; ++s) caps[s] = incb_capture_path(s);

    while (remaining) {
        size_t running = 0;
        if (!draining) {
            for (i = 0; i < incb_g.tcount; ++i) {
                Target* t = incb_g.order[i];
                size_t k;
                bool ready = true;
                if (st[i] != 0) continue;
                for (k = 0; t->kind != INCB_STATIC && k < t->dep_count; ++k)
                    if (st[t->deps[k]->order_index] != 2) ready = false;
                if (!ready) continue;
                for (s = 0; s < nslots && busy[s]; ++s) {}
                if (s == nslots) break;
                incb_mkdirs(incb_dirname(t->out_abs));
                incb_mkdirs(incb_dirname(t->cmdf_abs));
                if (t->kind == INCB_STATIC) remove(t->out_abs);   /* ar appends */
                ++started;
                incb_progress(started, total, t->kind == INCB_STATIC ? "ar " : "ld ", t->name);
                if (incb_g.verbose) {
                    incb_say("%s  $ %s%s", INCB_CDIM, t->link_cmd, INCB_COFF);
                    incb_say("%s  because: %s%s", INCB_CDIM, t->link_why, INCB_COFF);
                }
                if (!incb_spawn(t->link_cmd, caps[s], &procs[s]))
                    incb_fatal("cannot start the linker");
                busy[s] = true;
                payload[s] = t;
                st[i] = 1;
            }
        }
        for (s = 0; s < nslots; ++s) if (busy[s]) ++running;
        if (!running) break;
        {
            int code = 0;
            size_t hit = incb_wait_any(procs, busy, nslots, &code);
            Target* t = payload[hit];
            busy[hit] = false;
            incb_report(incb_fmt("linking %s", t->name), t->link_cmd, caps[hit], code);
            for (i = 0; i < incb_g.tcount; ++i) if (incb_g.order[i] == t) { st[i] = 2; break; }
            --remaining;
            if (code == 0) {
                incb_write_file(t->cmdf_abs, t->link_cmd);
                incb_stat_refresh(t->out_abs);
                ++*linked;
            } else {
                ++failures;
                draining = true;
            }
        }
    }
    return failures;
}

/* ------------------------------------------------------------------- verbs */

static void incb_json_str(FILE* f, const char* s) {
    const unsigned char* p = (const unsigned char*)s;
    fputc('"', f);
    for (; *p; ++p) {
        switch (*p) {
            case '\\': fputs("\\\\", f); break;
            case '"':  fputs("\\\"", f); break;
            case '\n': fputs("\\n", f); break;
            case '\r': fputs("\\r", f); break;
            case '\t': fputs("\\t", f); break;
            default:
                if (*p < 0x20) fprintf(f, "\\u%04x", *p);
                else fputc(*p, f);
        }
    }
    fputc('"', f);
}

static void incb_write_compdb(const char* path) {
    FILE* f;
    size_t i;
    incb_mkdirs(incb_dirname(path));
    f = fopen(path, "wb");
    if (!f) incb_fatal("cannot write '%s': %s", path, strerror(errno));
    fputs("[\n", f);
    for (i = 0; i < incb_g.acount; ++i) {
        const incb_act* a = &incb_g.acts[i];
        fputs("  {\n    \"directory\": ", f);
        incb_json_str(f, incb_g.root);
        fputs(",\n    \"command\": ", f);
        incb_json_str(f, a->cmd);
        fputs(",\n    \"file\": ", f);
        incb_json_str(f, a->src_abs);
        fputs(",\n    \"output\": ", f);
        incb_json_str(f, a->obj_abs);
        fputs("\n  }", f);
        if (i + 1 < incb_g.acount) fputc(',', f);
        fputc('\n', f);
    }
    fputs("]\n", f);
    if (fclose(f) != 0) incb_fatal("cannot write '%s': %s", path, strerror(errno));
}

static int incb_verb_build(void) {
    size_t compiled = 0, linked = 0, cfail, lfail = 0;
    double t0 = incb_now();
    char* compdb;
    incb_mkdirs(incb_fmt("%s%ctmp", incb_g.state_dir, INCB_SEP));
    incb_mkdirs(incb_g.out_dir);
    cfail = incb_compile_phase(&compiled);
    if (!cfail) lfail = incb_link_phase(&linked);
    if (cfail || lfail) {
        fprintf(stderr, "%sbuild failed%s (%u error%s)\n", INCB_CRED, INCB_COFF,
                (unsigned)(cfail + lfail), cfail + lfail == 1 ? "" : "s");
        return 1;
    }
    compdb = incb_join(incb_g.root, "compile_commands.json");
    if (incb_stat_fs(compdb) != 0) incb_write_compdb(compdb);  /* keep it fresh */
    if (compiled || linked) {
        incb_say("%sbuilt %u object%s, linked %u target%s in %.2fs (j=%d)%s",
                INCB_CGRN, (unsigned)compiled, compiled == 1 ? "" : "s",
                (unsigned)linked, linked == 1 ? "" : "s",
                incb_now() - t0, incb_g.jobs, INCB_COFF);
    } else {
        incb_say("%sall targets up to date%s", INCB_CGRN, INCB_COFF);
    }
    return 0;
}

static int incb_verb_run(void) {
    Target* t = NULL;
    size_t i;
    int rc;
    if (incb_g.run_target) {
        for (i = 0; i < incb_g.tcount; ++i)
            if (!strcmp(incb_g.targets[i]->name, incb_g.run_target)) t = incb_g.targets[i];
        if (!t) incb_fatal("run: no target named '%s'", incb_g.run_target);
        if (t->kind != INCB_EXE) incb_fatal("run: '%s' is not an executable", t->name);
    } else {
        for (i = 0; i < incb_g.tcount && !t; ++i)
            if (incb_g.targets[i]->kind == INCB_EXE) t = incb_g.targets[i];
        if (!t) incb_fatal("run: this project has no executable targets");
    }
    {
        for (i = 0; i < incb_g.tcount; ++i) incb_g.targets[i]->wanted = false;
        t->wanted = true;
        for (i = 0; i < t->dep_count; ++i) t->deps[i]->wanted = true;
    }
    rc = incb_verb_build();
    if (rc != 0) return rc;
    {
        incb_buf b = {NULL, 0, 0};
        incb_proc p;
        incb_add(&b, incb_quote(t->out_abs));
        for (i = 0; i < (size_t)incb_g.run_argc; ++i) {
            incb_addc(&b, ' ');
            incb_add(&b, incb_quote(incb_g.run_args[i]));
        }
        incb_say("%s$ %s%s", INCB_CDIM, b.data, INCB_COFF);
        if (!incb_spawn(incb_take(&b), NULL, &p))
            incb_fatal("run: cannot start %s", t->out_arg);
        return incb_proc_wait(&p);
    }
}

static int incb_verb_clean(void) {
    size_t i;
    incb_rmrf(incb_g.state_dir);
    for (i = 0; i < incb_g.tcount; ++i) incb_rmrf(incb_g.targets[i]->out_abs);
    incb_say("cleaned");
    return 0;
}

int build(void) {
    size_t i;
    if (!incb_g.inited) incb_fatal("build: call build_init(argc, argv) first");
    if (incb_g.built) incb_fatal("build may only be called once");
    incb_g.built = true;

    /* effective output settings (CLI overrides config) */
    incb_g.jobs = incb_g.cli_jobs > 0 ? incb_g.cli_jobs : (config.jobs > 0 ? config.jobs : incb_ncpu());
    if (incb_g.jobs < 1) incb_g.jobs = 1;
#ifdef _WIN32
    if (incb_g.jobs > 60) incb_g.jobs = 60;
#else
    if (incb_g.jobs > 512) incb_g.jobs = 512;
#endif
    incb_g.verbose = incb_g.cli_verbose >= 0 ? incb_g.cli_verbose != 0 : config.verbose != 0;
    incb_g.quiet = !incb_g.verbose &&
                  (incb_g.cli_quiet >= 0 ? incb_g.cli_quiet != 0 : config.quiet != 0);
    {
        int c = incb_g.cli_color >= 0 ? incb_g.cli_color : config.color;
        incb_g.color = c >= 0 ? c != 0 : (incb_tty(stdout) && !getenv("NO_COLOR"));
        if (incb_g.color) incb_color_on();
    }

    incb_self_rebuild();

    if (incb_g.verb == INCB_HELP) {
        fputs(incb_usage, stdout);
        if (incb_g.tcount) {
            printf("\ntargets in this project:\n");
            for (i = 0; i < incb_g.tcount; ++i) {
                Target* t = incb_g.targets[i];
                printf("  %-20s %s\n", t->name,
                       t->kind == INCB_EXE ? "executable" :
                       t->kind == INCB_STATIC ? "static library" : "shared library");
            }
        }
        return 0;
    }

    if (incb_g.verb != INCB_CLEAN && incb_g.tcount == 0)
        incb_fatal("no targets — declare one with executable/static_library/shared_library");

    incb_resolve_layout();
    if (incb_g.verb == INCB_CLEAN) return incb_verb_clean();

    incb_resolve_graph();
    incb_resolve_inputs();
    incb_plan_compile_actions();
    incb_plan_link_actions();

    switch (incb_g.verb) {
        case INCB_COMPDB:
            incb_write_compdb(incb_join(incb_g.root, "compile_commands.json"));
            incb_say("wrote compile_commands.json (it will stay fresh from now on)");
            return 0;
        case INCB_RUN:
            return incb_verb_run();
        case INCB_CLEAN:
            return 0; /* handled before planning */
        default:
            for (i = 0; i < incb_g.tcount; ++i) incb_g.targets[i]->wanted = true;
            return incb_verb_build();
    }
}

#endif /* BUILD_IMPLEMENTATION */
