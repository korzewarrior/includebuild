# IncludeBuild

Put `build.h` next to `build.c`.

```c
#define BUILD_IMPLEMENTATION
#include "build.h"

int main(int argc, char** argv) {
    build_init(argc, argv);
    executable("app", "**.c");
    return build();
}
```

```sh
cc -o build build.c
./build
./build release
./build run
./build clean
./build compdb
./build help
```

`./build` recompiles itself when `build.c` or `build.h` changes. It also checks
compiler depfiles and the command used for each object. `./build -v` prints the
commands and rebuild reasons.

## Targets

```c
Target* executable(const char* name, const char* pattern);
Target* static_library(const char* name, const char* pattern);
Target* shared_library(const char* name, const char* pattern);

void sources(Target* target, const char* pattern);
void include_dir(Target* target, const char* path);
void compile_flags(Target* target, const char* flags);
void link_flags(Target* target, const char* flags);
void use(Target* target, Target* library);
```

`*` and `?` match within a directory; `**` crosses directories. Repeated
patterns are sorted and deduplicated. A pattern that matches nothing is an
error.

## Configuration

```c
Target* app = executable("app", "src/**.c");
config.cc = "clang";
config.compile_flags = "-Wall -Wextra";
config.debug_flags = "-g -O0 -DDEBUG";
config.release_flags = "-O3 -DNDEBUG";
config.output_dir = "bin";

if (config.release) {
    compile_flags(app, "-DFAST_PATH");
}
```

`use(app, library)` adds the library, its dependencies, and its include
directories. Static libraries linked into shared libraries are compiled with
`-fPIC` on Linux and macOS.

## Requirements

C99. GCC, Clang, and MinGW are supported on Linux, macOS, and Windows. Compiler
defaults can be changed with `$CC`, `$CXX`, and `$AR`.

[Website](https://includebuild.com) ·
[Development repository](https://korze.org/code/includebuild) ·
[CC0](LICENSE)
