# IncludeBuild User Guide

This guide will help you get started with IncludeBuild and show you how to use it effectively in your C/C++ projects.

## Table of Contents

1. [Introduction](#introduction)
2. [Getting Started](#getting-started)
3. [Basic Usage](#basic-usage)
4. [Advanced Usage](#advanced-usage)
5. [Command Line Interface](#command-line-interface)
6. [Cross-Platform Considerations](#cross-platform-considerations)
7. [Best Practices](#best-practices)
8. [Troubleshooting](#troubleshooting)

## Introduction

IncludeBuild is a minimalist build system for C/C++ projects that's entirely contained in a single header file. It provides a programmatic C API for defining build processes, rather than requiring you to learn a separate build language.

## Getting Started

### Requirements

- A C compiler (GCC, Clang, MSVC, etc.)
- Standard C library

That's it! No other dependencies are required.

### Installation

You have two options for installing IncludeBuild:

#### Option 1: Copy to your project (recommended)

Simply copy the `build.h` file to your project directory:

```bash
cp /path/to/includebuild/src/build.h your_project/
```

#### Option 2: Install system-wide

On Linux/macOS:

```bash
cd /path/to/includebuild
sudo make install
```

This will install `build.h` to `/usr/local/include` by default.

## Basic Usage

### 1. Create a Build Script

Create a file named `main.c` in your project root:

```c
#include "build.h"

int main() {
    // Initialize IncludeBuild with default settings
    ib_init();
    
    // Build the project
    ib_build();
    
    return 0;
}
```

### 2. Compile the Build Script

```bash
gcc -o main main.c
```

### 3. Run the Build Script

```bash
./main
```

That's it! IncludeBuild will:
1. Scan your project directory for C/C++ source files
2. Parse header includes to determine dependencies
3. Compile source files that need to be built
4. Link all object files into an executable

## Advanced Usage

### Custom Configuration

You can customize various aspects of the build process:

```c
#include "build.h"

int main() {
    // Initialize IncludeBuild
    ib_init();
    
    // Customize build settings
    g_config.verbose = true;  // Enable verbose output
    strcpy(g_config.source_dir, "src");  // Source files in src/ directory
    strcpy(g_config.build_dir, "output");  // Output to output/ directory
    strcpy(g_config.compiler_flags, "-Wall -Wextra -O3 -std=c11");  // Compiler flags
    
    // Add include directories
    ib_add_include_dir("include");
    ib_add_include_dir("third_party/include");
    
    // Exclude files from the build
    strcpy(g_config.exclude_files[0], "main.c");
    g_config.num_exclude_files = 1;
    
    // Build the project
    ib_build();
    
    return 0;
}
```

### Custom Targets

You can define specific build targets:

```c
// Create an executable
ib_add_target("my_app", "src/main.c");

// Create a library
ib_add_target("my_lib", "src/lib.c");
```

If you don't specify any targets, IncludeBuild will create a default target based on the directory name and automatically find a suitable main source file.

## Command Line Interface

You can add command-line support to your build script:

```c
#include "build.h"
#include <string.h>

void print_usage(const char* program_name) {
    printf("Usage: %s [command]\n\n", program_name);
    printf("Commands:\n");
    printf("  build    Build the project (default)\n");
    printf("  clean    Clean the build directory\n");
    printf("  help     Show this help message\n");
}

int main(int argc, char **argv) {
    // Initialize IncludeBuild
    ib_init();
    
    // Parse command line arguments
    if (argc > 1) {
        if (strcmp(argv[1], "clean") == 0) {
            ib_clean();
            return 0;
        } else if (strcmp(argv[1], "help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[1], "build") != 0) {
            printf("Unknown command: %s\n\n", argv[1]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Build the project
    ib_build();
    
    return 0;
}
```

## Cross-Platform Considerations

IncludeBuild works on Linux, macOS, and Windows. It automatically handles platform differences, such as:

- Path separators (`/` on Unix, `\` on Windows)
- Compiler commands and flags
- Object file extensions (`.o` on Unix, `.obj` on Windows)

For cross-platform projects, you can add platform-specific logic in your build script:

```c
#include "build.h"

int main() {
    ib_init();
    
    #ifdef _WIN32
    // Windows-specific settings
    strcpy(g_config.compiler, "cl");
    strcpy(g_config.compiler_flags, "/nologo /W3 /O2");
    strcpy(g_config.linker_flags, "/SUBSYSTEM:CONSOLE");
    #else
    // Unix-specific settings
    strcpy(g_config.compiler, "gcc");
    strcpy(g_config.compiler_flags, "-Wall -Wextra -O2");
    strcpy(g_config.linker_flags, "-lm");
    #endif
    
    ib_build();
    
    return 0;
}
```

## Best Practices

### Project Structure

A typical project structure might look like this:

```
my_project/
  ├── src/              # Source files
  ├── include/          # Header files
  ├── tests/            # Test files
  ├── build             # IncludeBuild header
  ├── main.c           # Build script
  └── README.md         # Project documentation
```

### Build Script Organization

For complex projects, organize your build script into functions:

```c
#include "build.h"
#include <string.h>

void configure_build(void) {
    strcpy(g_config.source_dir, "src");
    strcpy(g_config.build_dir, "bin");
    g_config.verbose = true;
    ib_add_include_dir("include");
}

void add_targets(void) {
    ib_add_target("my_app", "src/main.c");
    ib_add_target("my_lib", "src/lib.c");
}

int main(int argc, char **argv) {
    ib_init();
    configure_build();
    add_targets();
    ib_build();
    return 0;
}
```

## Troubleshooting

### Common Issues

#### Build fails with "command not found"

Make sure the compiler specified in `g_config.compiler` is installed and in your PATH.

#### IncludeBuild doesn't find source files

Check that the source files are in the directory specified by `g_config.source_dir`. By default, this is the current directory.

#### Build fails with "multiple definition" errors

This can happen if multiple source files define the same symbol. You might need to:
- Move common code into header files
- Make functions static if they should only be used in one file
- Ensure you don't have multiple main functions in the build

#### Manually specifying dependencies

If IncludeBuild doesn't correctly determine dependencies (rare), you can modify the dependency graph in your build script. Check the API documentation for details.

### Getting Help

If you run into issues:

- Check the [FAQ](./faq.md) for common questions
- Read the [API Documentation](./api.md) for detailed information
- Open an issue on [GitHub](https://github.com/korzewarrior/includebuild/issues)

Remember, one benefit of IncludeBuild is that the entire system is in a single C header file, so you can read and modify it yourself if needed! 