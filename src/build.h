/**
 * IncludeBuild - A minimalist build system for C/C++ projects
 * 
 * Just #include "build.h" and you're ready to go
 * 
 * Basic usage:
 *   #include "build.h"
 *   
 *   int main() {
 *       ib_init();
 *       ib_build();
 *       return 0;
 *   }
 */

#ifndef INCLUDEBUILD_H
#define INCLUDEBUILD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEPARATOR '\\'
#define popen _popen
#define pclose _pclose
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/wait.h>
#define PATH_SEPARATOR '/'
#endif

// Version information
#define IB_VERSION_MAJOR 1
#define IB_VERSION_MINOR 0
#define IB_VERSION_PATCH 0

// Default limits (can be overridden at compile time)
#ifndef IB_MAX_PATH
#define IB_MAX_PATH 1024
#endif

#ifndef IB_MAX_CMD
#define IB_MAX_CMD 4096
#endif

#ifndef IB_MAX_FILES
#define IB_MAX_FILES 1000
#endif

#ifndef IB_MAX_DEPS
#define IB_MAX_DEPS 100
#endif

#ifndef IB_MAX_INCLUDE_DIRS
#define IB_MAX_INCLUDE_DIRS 50
#endif

#ifndef IB_MAX_TARGETS
#define IB_MAX_TARGETS 50
#endif

// Logging levels
typedef enum {
    IB_LOG_ERROR = 0,
    IB_LOG_WARN = 1,
    IB_LOG_INFO = 2,
    IB_LOG_DEBUG = 3
} ib_log_level;

// Forward declarations
typedef struct ib_file ib_file;
typedef struct ib_target ib_target;
typedef struct ib_config ib_config;

struct ib_file {
    char path[IB_MAX_PATH];             // Path to file
    char obj_path[IB_MAX_PATH];         // Path to output object file
    time_t last_modified;               // Last modified timestamp
    int num_deps;                       // Number of dependencies
    ib_file* deps[IB_MAX_DEPS];         // Dependencies
    bool needs_rebuild;                 // Whether file needs rebuilding
};

struct ib_target {
    char name[IB_MAX_PATH];             // Target name
    char output_path[IB_MAX_PATH];      // Output path
    char main_source[IB_MAX_PATH];      // Main source file
    int num_files;                      // Number of files in this target
    ib_file* files[IB_MAX_FILES];       // Files that make up this target
    bool is_library;                    // Whether this is a library
};

struct ib_config {
    char source_dir[IB_MAX_PATH];       // Source directory
    char build_dir[IB_MAX_PATH];        // Build directory
    char compiler[64];                  // Compiler to use
    char compiler_flags[IB_MAX_CMD];    // Compiler flags
    char linker_flags[IB_MAX_CMD];      // Linker flags
    char include_dirs[IB_MAX_INCLUDE_DIRS][IB_MAX_PATH]; // Include directories
    int num_include_dirs;               // Number of include directories
    char exclude_files[IB_MAX_FILES][IB_MAX_PATH]; // Files to exclude from build
    int num_exclude_files;              // Number of excluded files
    bool verbose;                       // Verbose output
    bool color_output;                  // Colorize output
    ib_log_level log_level;             // Current logging level
};

// Global state
static ib_config g_config;
static ib_file g_files[IB_MAX_FILES];
static int g_num_files = 0;
static ib_target g_targets[IB_MAX_TARGETS];
static int g_num_targets = 0;
static bool g_initialized = false;

// ANSI color codes
#define IB_COLOR_RESET   "\x1b[0m"
#define IB_COLOR_RED     "\x1b[31m"
#define IB_COLOR_GREEN   "\x1b[32m"
#define IB_COLOR_YELLOW  "\x1b[33m"
#define IB_COLOR_BLUE    "\x1b[34m"
#define IB_COLOR_MAGENTA "\x1b[35m"
#define IB_COLOR_CYAN    "\x1b[36m"

// Helper macros for colorized output
#define IB_COLOR(color, text) (g_config.color_output ? ((color) text IB_COLOR_RESET) : (text))

// Function declarations
static void ib_log_message(ib_log_level level, const char* fmt, ...);
static void ib_error(const char* fmt, ...);
static void ib_warning(const char* fmt, ...);
static bool ib_file_exists(const char* path);
static time_t ib_get_file_mtime(const char* path);
static void ib_ensure_dir_exists(const char* path);
static char* ib_join_path(char* dest, const char* path1, const char* path2);
static void ib_find_source_files(const char* dir);
static void ib_parse_dependencies(ib_file* file);
static bool ib_needs_rebuild(ib_file* file);
static void ib_compile_file(ib_file* file);
static void ib_link_target(ib_target* target);
static void ib_add_default_target(void);

/**
 * Initialize IncludeBuild with default configuration
 */
void ib_init(void) {
    if (g_initialized) {
        ib_error("IncludeBuild already initialized");
        return;
    }
    
    // Set default configuration
    memset(&g_config, 0, sizeof(g_config));
    strcpy(g_config.source_dir, ".");
    strcpy(g_config.build_dir, "build");
    
#ifdef _WIN32
    strcpy(g_config.compiler, "cl");
    strcpy(g_config.compiler_flags, "/nologo /W3 /O2");
#else
    strcpy(g_config.compiler, "gcc");
    strcpy(g_config.compiler_flags, "-Wall -Wextra -O2");
#endif
    
    g_config.verbose = false;
    g_config.color_output = true;
    g_config.num_exclude_files = 0;
    g_config.log_level = IB_LOG_INFO;
    
    // Add current directory to include dirs
    strcpy(g_config.include_dirs[0], ".");
    g_config.num_include_dirs = 1;
    
    g_initialized = true;
    
    ib_log_message(IB_LOG_INFO, "IncludeBuild v%d.%d.%d initialized", 
        IB_VERSION_MAJOR, IB_VERSION_MINOR, IB_VERSION_PATCH);
}

/**
 * Initialize IncludeBuild with custom configuration
 */
void ib_init_with_config(const ib_config* config) {
    if (g_initialized) {
        ib_error("IncludeBuild already initialized");
        return;
    }
    
    // Copy config
    memcpy(&g_config, config, sizeof(ib_config));
    
    g_initialized = true;
    
    ib_log_message(IB_LOG_INFO, "IncludeBuild v%d.%d.%d initialized with custom config", 
        IB_VERSION_MAJOR, IB_VERSION_MINOR, IB_VERSION_PATCH);
}

/**
 * Add an include directory with validation
 */
void ib_add_include_dir(const char* dir) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return;
    }
    
    if (!dir || strlen(dir) == 0) {
        ib_error("Invalid directory path (null or empty)");
        return;
    }
    
    if (g_config.num_include_dirs >= IB_MAX_INCLUDE_DIRS) {
        ib_error("Too many include directories (max: %d)", IB_MAX_INCLUDE_DIRS);
        return;
    }
    
    // Check if directory exists
    if (!ib_file_exists(dir)) {
        ib_warning("Include directory does not exist: %s", dir);
    }
    
    strcpy(g_config.include_dirs[g_config.num_include_dirs++], dir);
    ib_log_message(IB_LOG_DEBUG, "Added include directory: %s", dir);
}

/**
 * Add a build target
 */
void ib_add_target(const char* name, const char* main_source) {
    if (g_num_targets >= IB_MAX_TARGETS) {
        ib_error("Too many targets (max: %d)", IB_MAX_TARGETS);
        return;
    }
    
    ib_target* target = &g_targets[g_num_targets++];
    strcpy(target->name, name);
    
    // Set output path
    ib_join_path(target->output_path, g_config.build_dir, name);
    
    #ifdef _WIN32
    strcat(target->output_path, ".exe");
    #endif
    
    strcpy(target->main_source, main_source);
    target->num_files = 0;
    target->is_library = false;
}

/**
 * Build the project
 */
bool ib_build(void) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return false;
    }
    
    ib_log_message(IB_LOG_INFO, "Building project...");
    
    // Create build directory if it doesn't exist
    ib_ensure_dir_exists(g_config.build_dir);
    
    // Find all source files
    ib_find_source_files(g_config.source_dir);
    
    // If no targets defined, create a default one
    if (g_num_targets == 0) {
        ib_add_default_target();
    }
    
    // Parse dependencies for all files
    for (int i = 0; i < g_num_files; i++) {
        ib_parse_dependencies(&g_files[i]);
    }
    
    // Compile all files that need rebuilding
    int num_compiled = 0;
    for (int i = 0; i < g_num_files; i++) {
        if (ib_needs_rebuild(&g_files[i])) {
            ib_compile_file(&g_files[i]);
            num_compiled++;
        }
    }
    
    // Link all targets
    for (int i = 0; i < g_num_targets; i++) {
        ib_link_target(&g_targets[i]);
    }
    
    ib_log_message(IB_LOG_INFO, "Build complete. Compiled %d files.", num_compiled);
    return true;
}

/**
 * Clean the build directory
 */
void ib_clean(void) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return;
    }
    
    ib_log_message(IB_LOG_INFO, "Cleaning build directory: %s", g_config.build_dir);
    
    // Clean the build directory by removing all object files and target executables
    DIR* d = opendir(g_config.build_dir);
    if (!d) {
        ib_error("Failed to open build directory: %s", g_config.build_dir);
        return;
    }
    
    struct dirent* entry;
    int num_removed = 0;
    
    while ((entry = readdir(d)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Check file extension
        const char* ext = strrchr(entry->d_name, '.');
        bool is_obj = false;
        if (ext) {
            #ifdef _WIN32
            is_obj = (strcmp(ext, ".obj") == 0);
            #else
            is_obj = (strcmp(ext, ".o") == 0);
            #endif
        }
        
        // If it's an object file or a target executable (no extension), remove it
        if (is_obj || ext == NULL) {
            char path[IB_MAX_PATH];
            ib_join_path(path, g_config.build_dir, entry->d_name);
            
            if (g_config.verbose) {
                ib_log_message(IB_LOG_INFO, "  Removing %s", path);
            }
            
            if (remove(path) == 0) {
                num_removed++;
            } else {
                ib_error("Failed to remove file: %s (%s)", path, strerror(errno));
            }
        }
    }
    
    closedir(d);
    
    ib_log_message(IB_LOG_INFO, "Clean complete. Removed %d files.", num_removed);
}

/**
 * Set the logging level
 */
void ib_set_log_level(ib_log_level level) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return;
    }
    
    g_config.log_level = level;
}

/**
 * Log a message with the specified level
 */
static void ib_log_message(ib_log_level level, const char* fmt, ...) {
    // Only print if our level is sufficient
    if (level > g_config.log_level && level != IB_LOG_ERROR) {
        return;
    }
    
    const char* prefix = "";
    const char* color = "";
    
    switch (level) {
        case IB_LOG_ERROR:
            prefix = "[ERROR] ";
            color = IB_COLOR_RED;
            break;
        case IB_LOG_WARN:
            prefix = "[WARN] ";
            color = IB_COLOR_YELLOW;
            break;
        case IB_LOG_INFO:
            prefix = "[INFO] ";
            color = IB_COLOR_GREEN;
            break;
        case IB_LOG_DEBUG:
            prefix = "[DEBUG] ";
            color = IB_COLOR_CYAN;
            break;
    }
    
    fprintf(stdout, "%s", IB_COLOR(color, prefix));
    
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    
    fprintf(stdout, "\n");
}

/**
 * Log an error message
 */
static void ib_error(const char* fmt, ...) {
    fprintf(stderr, "%s", IB_COLOR(IB_COLOR_RED, "[ERROR] "));
    
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    
    fprintf(stderr, "\n");
}

/**
 * Log a warning message
 */
static void ib_warning(const char* fmt, ...) {
    if (g_config.log_level < IB_LOG_WARN) {
        return;
    }
    
    fprintf(stdout, "%s", IB_COLOR(IB_COLOR_YELLOW, "[WARN] "));
    
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    
    fprintf(stdout, "\n");
}

/**
 * Check if a file exists
 */
static bool ib_file_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/**
 * Get the last modified time of a file
 */
static time_t ib_get_file_mtime(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_mtime;
    }
    return 0;
}

/**
 * Ensure a directory exists, creating it if necessary
 */
static void ib_ensure_dir_exists(const char* path) {
    if (ib_file_exists(path)) {
        return;
    }
    
    ib_log_message(IB_LOG_INFO, "Creating directory: %s", path);
    
    // Create the directory
    #ifdef _WIN32
    if (mkdir(path) != 0) {
    #else
    if (mkdir(path, 0755) != 0) {
    #endif
        ib_error("Failed to create directory: %s (%s)", path, strerror(errno));
        // Exit since we can't continue without the build directory
        exit(1);
    }
}

/**
 * Join two paths together
 */
static char* ib_join_path(char* dest, const char* path1, const char* path2) {
    strcpy(dest, path1);
    
    // Add separator if needed
    size_t len = strlen(dest);
    if (len > 0 && dest[len - 1] != PATH_SEPARATOR && path2[0] != PATH_SEPARATOR) {
        dest[len] = PATH_SEPARATOR;
        dest[len + 1] = '\0';
    } else if (len > 0 && dest[len - 1] == PATH_SEPARATOR && path2[0] == PATH_SEPARATOR) {
        dest[len - 1] = '\0';
    }
    
    strcat(dest, path2);
    return dest;
}

/**
 * Find all source files in a directory
 */
static void ib_find_source_files(const char* dir) {
    DIR* d = opendir(dir);
    if (!d) {
        ib_error("Failed to open directory: %s", dir);
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(d)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char path[IB_MAX_PATH];
        ib_join_path(path, dir, entry->d_name);
        
        struct stat st;
        if (stat(path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // Recursively scan subdirectory
                ib_find_source_files(path);
            } else if (S_ISREG(st.st_mode)) {
                // Check if it's a C/C++ source file
                const char* ext = strrchr(entry->d_name, '.');
                if (ext && (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 || 
                           strcmp(ext, ".cc") == 0 || strcmp(ext, ".cxx") == 0)) {
                    
                    // Check if this file is in the exclude list
                    bool excluded = false;
                    for (int i = 0; i < g_config.num_exclude_files; i++) {
                        if (strstr(path, g_config.exclude_files[i]) != NULL) {
                            if (g_config.verbose) {
                                ib_log_message(IB_LOG_INFO, "Excluding file: %s", path);
                            }
                            excluded = true;
                            break;
                        }
                    }
                    
                    if (excluded) {
                        continue;
                    }
                    
                    if (g_num_files >= IB_MAX_FILES) {
                        ib_error("Too many source files (max: %d)", IB_MAX_FILES);
                        break;
                    }
                    
                    // Add file to the list
                    ib_file* file = &g_files[g_num_files++];
                    strcpy(file->path, path);
                    
                    // Set object file path
                    char rel_path[IB_MAX_PATH];
                    if (strncmp(path, g_config.source_dir, strlen(g_config.source_dir)) == 0) {
                        strcpy(rel_path, path + strlen(g_config.source_dir));
                        if (rel_path[0] == PATH_SEPARATOR) {
                            memmove(rel_path, rel_path + 1, strlen(rel_path));
                        }
                    } else {
                        strcpy(rel_path, entry->d_name);
                    }
                    
                    char obj_name[IB_MAX_PATH];
                    strcpy(obj_name, rel_path);
                    char* dot = strrchr(obj_name, '.');
                    if (dot) {
                        #ifdef _WIN32
                        strcpy(dot, ".obj");
                        #else
                        strcpy(dot, ".o");
                        #endif
                    }
                    
                    // Replace path separators with underscores in object name
                    for (char* p = obj_name; *p; p++) {
                        if (*p == PATH_SEPARATOR) {
                            *p = '_';
                        }
                    }
                    
                    ib_join_path(file->obj_path, g_config.build_dir, obj_name);
                    
                    file->last_modified = st.st_mtime;
                    file->num_deps = 0;
                    file->needs_rebuild = true;
                    
                    if (g_config.verbose) {
                        ib_log_message(IB_LOG_INFO, "Found source file: %s", file->path);
                    }
                }
            }
        }
    }
    
    closedir(d);
}

/**
 * Parse dependencies for a file
 */
static void ib_parse_dependencies(ib_file* file) {
    // Open the file
    FILE* fp = fopen(file->path, "r");
    if (!fp) {
        ib_error("Failed to open file: %s", file->path);
        return;
    }
    
    // Buffer for reading lines
    char line[IB_MAX_PATH];
    
    // Check each line for includes
    while (fgets(line, sizeof(line), fp)) {
        // Look for #include "..."
        char* include_start = strstr(line, "#include \"");
        if (include_start) {
            include_start += 10; // Skip "#include \""
            char* include_end = strchr(include_start, '"');
            if (include_end) {
                *include_end = '\0';
                
                // Search for the included file in include dirs
                for (int i = 0; i < g_config.num_include_dirs; i++) {
                    char include_path[IB_MAX_PATH];
                    ib_join_path(include_path, g_config.include_dirs[i], include_start);
                    
                    // Check if this file exists
                    if (ib_file_exists(include_path)) {
                        // Look for this file in our file list
                        bool found = false;
                        for (int j = 0; j < g_num_files; j++) {
                            if (strcmp(g_files[j].path, include_path) == 0) {
                                // Add as dependency if not already added
                                bool already_dep = false;
                                for (int k = 0; k < file->num_deps; k++) {
                                    if (file->deps[k] == &g_files[j]) {
                                        already_dep = true;
                                        break;
                                    }
                                }
                                
                                if (!already_dep && file->num_deps < IB_MAX_DEPS) {
                                    file->deps[file->num_deps++] = &g_files[j];
                                    if (g_config.verbose) {
                                        ib_log_message(IB_LOG_INFO, "  Dependency: %s -> %s", file->path, include_path);
                                    }
                                }
                                
                                found = true;
                                break;
                            }
                        }
                        
                        if (found) {
                            break;
                        }
                    }
                }
            }
        }
    }
    
    fclose(fp);
}

/**
 * Check if a file needs to be rebuilt
 */
static bool ib_needs_rebuild(ib_file* file) {
    // If object file doesn't exist, rebuild
    if (!ib_file_exists(file->obj_path)) {
        return true;
    }
    
    // Get object file's modification time
    time_t obj_mtime = ib_get_file_mtime(file->obj_path);
    
    // If source file is newer than object file, rebuild
    if (file->last_modified > obj_mtime) {
        return true;
    }
    
    // Check if any dependencies are newer
    for (int i = 0; i < file->num_deps; i++) {
        if (ib_needs_rebuild(file->deps[i]) || file->deps[i]->last_modified > obj_mtime) {
            return true;
        }
    }
    
    return false;
}

/**
 * Compile a source file
 */
static void ib_compile_file(ib_file* file) {
    ib_log_message(IB_LOG_INFO, "Compiling %s", file->path);
    
    char cmd[IB_MAX_CMD];
    int include_flags_len = 0;
    char include_flags[IB_MAX_CMD] = "";
    
    // Build include flags
    for (int i = 0; i < g_config.num_include_dirs; i++) {
        int dir_len = strlen(g_config.include_dirs[i]);
        
        // Check if we would exceed the buffer
        if (include_flags_len + dir_len + 3 >= IB_MAX_CMD) {
            ib_warning("Include flags too long, some directories will be omitted");
            break;
        }
        
#ifdef _WIN32
        include_flags_len += snprintf(include_flags + include_flags_len, IB_MAX_CMD - include_flags_len,
                                     "/I\"%s\" ", g_config.include_dirs[i]);
#else
        include_flags_len += snprintf(include_flags + include_flags_len, IB_MAX_CMD - include_flags_len,
                                     "-I\"%s\" ", g_config.include_dirs[i]);
#endif
    }
    
    // Ensure build directory exists
    char obj_dir[IB_MAX_PATH] = {0};
    strcpy(obj_dir, file->obj_path);
    
    // Get directory part
    char* last_sep = strrchr(obj_dir, PATH_SEPARATOR);
    if (last_sep) {
        *last_sep = '\0';
        ib_ensure_dir_exists(obj_dir);
    }
    
    // Build command
    sprintf(cmd, "%s %s -c %s -o %s", 
        g_config.compiler, g_config.compiler_flags, file->path, file->obj_path);
    
    if (g_config.verbose) {
        ib_log_message(IB_LOG_INFO, "  Command: %s", cmd);
    }
    
    // Execute the command
    FILE* proc = popen(cmd, "r");
    if (!proc) {
        ib_error("Failed to execute command: %s", cmd);
        return;
    }
    
    // Read output
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), proc)) {
        if (g_config.verbose) {
            printf("%s", buffer);
        }
    }
    
    // Check result
    int result = pclose(proc);
    if (result != 0) {
        ib_error("Compilation failed with code %d", result);
    } else {
        file->needs_rebuild = false;
    }
}

/**
 * Link a build target
 */
static void ib_link_target(ib_target* target) {
    ib_log_message(IB_LOG_INFO, "Linking %s", target->name);
    
    // Find all files that belong to this target
    char obj_files[IB_MAX_CMD] = "";
    
    // Create a list of object files (avoiding duplicates)
    bool obj_included[IB_MAX_FILES] = {false};
    
    // Start with the main source file if specified
    if (target->main_source[0]) {
        // Find the corresponding object file
        for (int i = 0; i < g_num_files; i++) {
            if (strcmp(g_files[i].path, target->main_source) == 0) {
                strcat(obj_files, " ");
                strcat(obj_files, g_files[i].obj_path);
                obj_included[i] = true;
                break;
            }
        }
    }
    
    // Add all other object files (avoiding duplicates)
    for (int i = 0; i < g_num_files; i++) {
        if (!obj_included[i] && g_files[i].path[0]) {
            strcat(obj_files, " ");
            strcat(obj_files, g_files[i].obj_path);
        }
    }
    
    // Build command
    char cmd[IB_MAX_CMD];
    
    #ifdef _WIN32
    // MSVC command
    sprintf(cmd, "%s %s /Fe%s %s %s", 
        g_config.compiler, g_config.compiler_flags, target->output_path, obj_files, g_config.linker_flags);
    #else
    // GCC/Clang command
    sprintf(cmd, "%s %s -o %s %s %s", 
        g_config.compiler, g_config.compiler_flags, target->output_path, obj_files, g_config.linker_flags);
    #endif
    
    if (g_config.verbose) {
        ib_log_message(IB_LOG_INFO, "  Command: %s", cmd);
    }
    
    // Execute the command
    FILE* proc = popen(cmd, "r");
    if (!proc) {
        ib_error("Failed to execute command: %s", cmd);
        return;
    }
    
    // Read output
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), proc)) {
        if (g_config.verbose) {
            printf("%s", buffer);
        }
    }
    
    // Check result
    int result = pclose(proc);
    if (result != 0) {
        ib_error("Linking failed with code %d", result);
    } else {
        ib_log_message(IB_LOG_INFO, "Created %s", target->output_path);
    }
}

/**
 * Add a default target based on source files
 */
static void ib_add_default_target(void) {
    if (g_num_files == 0) {
        ib_error("No source files found");
        return;
    }
    
    // Look for a main.c or similar
    const char* main_candidates[] = {
        "main.c", "main.cpp", "main.cc", "Main.c", "Main.cpp",
        "app.c", "app.cpp", "Application.c", "Application.cpp",
        NULL
    };
    
    const char* main_file = NULL;
    
    // First try to find any of the candidates in the root directory
    for (const char** candidate = main_candidates; *candidate; candidate++) {
        char path[IB_MAX_PATH];
        ib_join_path(path, g_config.source_dir, *candidate);
        
        if (ib_file_exists(path)) {
            main_file = path;
            break;
        }
    }
    
    // If not found, look through all files
    if (!main_file) {
        for (int i = 0; i < g_num_files; i++) {
            const char* filename = strrchr(g_files[i].path, PATH_SEPARATOR);
            if (filename) {
                filename++; // Skip the separator
            } else {
                filename = g_files[i].path;
            }
            
            for (const char** candidate = main_candidates; *candidate; candidate++) {
                if (strcmp(filename, *candidate) == 0) {
                    main_file = g_files[i].path;
                    break;
                }
            }
            
            if (main_file) {
                break;
            }
        }
    }
    
    // Create a default target
    const char* target_name;
    char dir_name[IB_MAX_PATH];
    
    // Use directory name as target name
    strcpy(dir_name, g_config.source_dir);
    char* last_sep = strrchr(dir_name, PATH_SEPARATOR);
    if (last_sep) {
        target_name = last_sep + 1;
    } else {
        target_name = dir_name;
    }
    
    // If it's an empty string or just ".", use "app"
    if (!*target_name || strcmp(target_name, ".") == 0) {
        target_name = "app";
    }
    
    ib_add_target(target_name, main_file ? main_file : "");
    ib_log_message(IB_LOG_INFO, "Created default target: %s", target_name);
}

#endif /* INCLUDEBUILD_H */ 