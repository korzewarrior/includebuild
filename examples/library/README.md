# IncludeBuild Library Example - Magic Edition

## The Two Line Build System

IncludeBuild makes creating libraries so simple, the entire build script is just **two lines of code**:

```c
ib_init();
ib_build_library("logger");
```

That's it. With just those two lines, IncludeBuild automatically:

- Finds all source files in the `src/` directory
- Creates a static library (`lib/liblogger.a`)
- Creates a dynamic/shared library (`lib/liblogger.so`)
- Builds test programs (any file starting with `test_`)
- Creates helper scripts for running tests with the library path set
- Processes command-line arguments (clean, static, dynamic, test)

## IncludeBuild Philosophy

IncludeBuild is designed with these principles:

1. **Simplicity First** - The simplest solution is often the best
2. **Transparency** - No magic black boxes, just straightforward C code
3. **Minimal Configuration** - Reasonable defaults that just work
4. **Just Drop It In** - Include one header file and you're ready to go

## Directory Structure

For the library example, we use this structure:

```
library/
├── build.c            <- The two-line build script
├── include/
│   └── logger.h       <- Public header file for the library
├── src/
│   └── logger.c       <- Implementation of the library
└── test_logger.c      <- Tests for the library
```

## Building the Example

1. Compile the build script:
   ```
   gcc -o build build.c
   ```

2. Build everything (static library, dynamic library, and tests):
   ```
   ./build
   ```

3. Or build specific targets:
   ```
   ./build static    # Build only the static library
   ./build dynamic   # Build only the dynamic library
   ./build test      # Build only the test program
   ./build clean     # Clean all build artifacts
   ```

## How The Magic Works

The `ib_build_library()` function handles everything automatically:

- It looks for source files in the `src/` directory
- It assumes your public headers are in `include/`
- It creates both static and dynamic libraries with standard naming (`lib{name}.a` and `lib{name}.so`)
- It detects and builds any test files (those starting with `test_`)
- It automatically creates helper scripts for testing
- It handles command-line arguments

## Example Usage

The example includes a simple logging library with these features:

```c
#include "logger.h"

// Initialize with output file (NULL for stdout)
logger_init(NULL);

// Log messages at different levels
log_info("Program started");
log_debug("Processing item %d", item_id);
log_warning("Missing configuration file");
log_error("Failed to open file: %s", filename);

// Redirect output to a file
logger_set_output("app.log");
```

## Troubleshooting

If you encounter issues:

- Make sure you have the build directory structure set up correctly
- Check that you're calling `ib_init()` before `ib_build_library()`
- Ensure gcc/clang is installed on your system
- For test programs, make sure they include the library's header files

## Further Reading

- See the main IncludeBuild documentation for more options
- Check other examples for different project types
- Learn how to customize the build process for specific needs

## What is IncludeBuild?

IncludeBuild is a minimalist, header-only build system for C/C++ projects that's included directly in your code. Unlike complex external build systems like Make, CMake, or Meson, IncludeBuild allows you to define your build process directly in your C code.

The core philosophy is that building software should be **simple**, **transparent**, and **embedded in the code itself**.

## What This Example Demonstrates

This example shows how IncludeBuild can handle complex tasks like library creation with almost no code, following a simple convention-based approach:

1. Put your header files in `include/`
2. Put your source files in `src/`
3. Name test files with a `test_` prefix
4. Let IncludeBuild do all the work

## Directory Structure

```
library/
├── include/           # Public header files
│   └── logger.h       # Logger API header
├── src/               # Implementation files
│   └── logger.c       # Logger implementation
├── test_logger.c      # Test program that uses the library
├── build.c            # The magic 2-line build script
└── README.md          # This file
```

## Getting Started

### 1. Compile the build script

```bash
gcc -o build build.c
```

### 2. Run the build script

```bash
./build  # Builds everything
```

IncludeBuild also automatically provides these commands:

```bash
./build static   # Build only static library
./build dynamic  # Build only dynamic library
./build test     # Build only test programs
./build clean    # Clean all build artifacts
./build help     # Show help message
```

### 3. Run the test program

```bash
# Use the generated helper script
./run_test_logger.sh
```

## How The Magic Works

The magic is in the `ib_build_library()` function, which:

1. **Auto-discovers** your source files in the `src/` directory
2. **Auto-configures** compiler and linker flags for libraries
3. **Auto-detects** test programs that start with `test_`
4. **Auto-generates** helper scripts for running with the correct library path
5. **Auto-handles** command-line arguments without any extra code
6. **Auto-excludes** test files from library compilation
7. **Auto-creates** necessary directories (`lib/`, etc.)

All of this happens with zero configuration required from the user. 

## Understanding the Logger Library

To show how IncludeBuild can be used with actual code, this example implements a simple logging library with these features:

1. **Log levels**: DEBUG, INFO, WARNING, ERROR, FATAL
2. **Formatted output**: Timestamp, log level, and message
3. **Console or file output**: Redirect logs to a file or stdout
4. **Message formatting**: Support for printf-style format specifiers

### Example Usage

```c
#include "logger.h"

int main() {
    // Initialize with minimum level
    logger_init(LOG_DEBUG);
    
    // Log messages at different levels
    logger_debug("Debug message");
    logger_info("Information: %s", "important info");
    logger_warning("Warning: temperature is %d°C", 85);
    logger_error("Error code: %d", 404);
    
    // Log to a file
    logger_set_output_file("application.log");
    logger_info("Logging to file now");
    
    // Clean up
    logger_cleanup();
    return 0;
}
```

## Make Your Own Library in 3 Steps

1. Create the directory structure (`include/`, `src/`, and a test file)
2. Copy the 5-line build.c and change "logger" to your library name
3. Run it!

That's it. No build systems to learn, no configuration files to write, no external dependencies.

## The Philosophy

IncludeBuild's "Library Magic" follows these principles:

1. **Convention over Configuration**: Follow a simple directory structure and naming convention
2. **Zero Boilerplate**: The minimal viable build script should be just a few lines
3. **Sane Defaults**: Everything should "just work" out of the box
4. **Transparency**: Clear output shows what's happening behind the scenes
5. **No External Dependencies**: Everything is contained in a single header file

## Troubleshooting

If you encounter issues:

- Make sure your directory structure follows the convention (`include/`, `src/`)
- Test files should be named with a `test_` prefix
- Header files should be in the `include/` directory
- For the generated run scripts to work, you'll need a Unix-like environment

## Further Reading

For more information on IncludeBuild and its features:

- Check the main IncludeBuild documentation in the root directory
- See other examples for different use cases
- Review the API documentation for more advanced customization options

---

This example demonstrates the power of IncludeBuild's convention-based approach to building libraries with minimal code. 