# Multi-File Example with IncludeBuild

This example demonstrates how to build a C project with multiple source files using IncludeBuild.

## Files in this example:

- `math_utils.h` & `math_utils.c`: Math utility functions
- `string_utils.h` & `string_utils.c`: String utility functions
- `main.c`: Main program that uses both utilities
- `build.c`: Build script that compiles the project

## Building the example

1. Compile the build script:
   ```bash
   gcc -o build build.c
   ```

2. Run the build script:
   ```bash
   ./build
   ```

This will compile all source files and link them together into an executable called "multi_file".

## How it works

The build script:
1. Adds all source files (except main.c)
2. Sets up the proper include paths
3. Configures compiler flags
4. Builds the project 

The example demonstrates:
- How IncludeBuild handles multiple source files
- Dependency detection (when you modify a header, all files that include it get rebuilt)
- Incremental builds (only changed files are recompiled) 