# IncludeBuild: Frequently Asked Questions

## General Questions

### What is IncludeBuild?

IncludeBuild is a header-only build system for C/C++ projects. It allows you to define your build process directly in your C/C++ code by including a single header file. This eliminates the need for separate build configuration files like Makefiles or CMakeLists.txt.

### Why would I use IncludeBuild instead of Make, CMake, or other build systems?
IncludeBuild has several advantages:
- **Simplicity**: It's a single header file with no dependencies
- **Programmability**: You write your build script in C, a language you already know
- **No DSL**: No need to learn a domain-specific language
- **Automatic dependency resolution**: Automatically finds source files and resolves dependencies
- **Cross-platform**: Works on Linux, macOS, and Windows without configuration
- **Control**: You have full programmatic control of your build process

### Does IncludeBuild replace IDEs?
No, IncludeBuild works alongside IDEs. In fact, many IDEs can run build scripts directly. IncludeBuild handles the compilation and linking of your code, while IDEs provide editing, debugging, and other features.

### Is IncludeBuild suitable for large projects?
IncludeBuild works well for small to medium-sized projects. For very large projects with complex dependencies and build requirements, you might find that more feature-rich build systems like CMake provide better tools for managing complexity. However, many seemingly complex builds can be simplified with a good project structure, which IncludeBuild encourages.

## Installation Questions

### How do I install IncludeBuild?

You have two options:

1. **Copy to your project**: Simply copy the `build.h` file from the `src` directory to your project.
2. **System-wide installation**: Run `make install` in the IncludeBuild repository to install it to your system's include path.

### Do I need to compile or build IncludeBuild itself?

No. IncludeBuild is header-only, which means you just need to include it in your code. There's no need to compile or build IncludeBuild itself.

### Can I use IncludeBuild with IDEs?

Yes. Since IncludeBuild is just a C header file, it works with any IDE that supports C/C++ development. Simply include the header in your project and compile your build script like any other C program.

## Usage Questions

### How do I create a basic build script?

Create a file called `build.c` with the following content:

```c
#include "build.h"

int main() {
    ib_workspace_t ws = ib_create_workspace();
    ib_add_source_file(ws, "main.c");
    ib_set_output(ws, "my_program");
    ib_build(ws);
    ib_destroy_workspace(ws);
    return 0;
}
```

Compile and run it:

```bash
gcc -o build build.c
./build
```

### How do I add multiple source files?

You can add them individually:

```c
ib_add_source_file(ws, "file1.c");
ib_add_source_file(ws, "file2.c");
ib_add_source_file(ws, "file3.c");
```

Or add all files in a directory:

```c
ib_add_source_directory(ws, "src", true); // true for recursive
```

### How do I link against libraries?

Use the library functions:

```c
// Add a library search directory
ib_add_library_dir(ws, "/usr/local/lib");

// Link against libraries
ib_add_library(ws, "m");       // Math library
ib_add_library(ws, "pthread"); // Pthread library
```

### How do I specify compiler or linker flags?

Use the flag functions:

```c
// Compiler flags
ib_add_compiler_flag(ws, "-Wall");
ib_add_compiler_flag(ws, "-O2");

// Linker flags
ib_add_linker_flag(ws, "-static");
```

### Can I use a different compiler?

Yes, you can specify which compiler to use:

```c
ib_set_compiler_executable(ws, "clang");
ib_set_linker_executable(ws, "clang");
```

## Technical Questions

### How does IncludeBuild work internally?

IncludeBuild analyzes your source files, constructs the appropriate compiler and linker commands, and executes them to build your project. It handles dependency analysis, incremental builds, and command generation automatically.

### What are the size limitations of IncludeBuild?

By default, IncludeBuild has the following limits:
- Maximum source files: 500
- Maximum include directories: 50
- Maximum library directories: 20

You can override these limits by defining macros before including the header:

```c
#define IB_MAX_SOURCE_FILES 1000
#define IB_MAX_INCLUDE_DIRS 100
#define IB_MAX_LIBRARY_DIRS 50
#include "build.h"
```

### Does IncludeBuild support incremental builds?

Yes. IncludeBuild checks file timestamps and only rebuilds files that have changed or depend on files that have changed.

### How do I clean build artifacts?

Use the clean function:

```c
ib_clean(ws);
```

### Does IncludeBuild work on all platforms?

IncludeBuild is designed to work on Linux, macOS, and Windows. The core functionality should work across all these platforms, but some platform-specific features may vary.

## Troubleshooting

### I'm getting "file not found" errors for standard headers

Make sure your compiler is correctly installed and that the standard include paths are set up properly. On some systems, you may need to install development packages (e.g., `build-essential` on Debian/Ubuntu).

### My build fails without a clear error message

Always check the return value of `ib_build()` and use `ib_get_last_error()` to get detailed error information:

```c
if (!ib_build(ws)) {
    printf("Build failed: %s\n", ib_get_last_error(ws));
    ib_destroy_workspace(ws);
    return 1;
}
```

### Can I see more detailed build output?

Yes, you can increase the log level:

```c
ib_set_log_level(ws, IB_LOG_DEBUG);
```

### IncludeBuild doesn't find my compiler

Ensure the compiler is in your PATH, or specify the full path to the compiler:

```c
ib_set_compiler_executable(ws, "/path/to/gcc");
```

## Support and Contribution

### How do I report a bug?

Open an issue on our GitHub repository with a detailed description of the bug, including steps to reproduce, expected behavior, and actual behavior.

### Can I contribute to IncludeBuild?

Absolutely! See the [CONTRIBUTING.md](../CONTRIBUTING.md) file for guidelines on how to contribute.

### Where can I get help if my question isn't answered here?

- Check the [User Guide](guide.md) and [API Reference](api.md)
- Look through the [examples](../examples/)
- Open an issue on our GitHub repository 