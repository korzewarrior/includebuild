# Basic Example with IncludeBuild

This is the simplest example of using IncludeBuild. It demonstrates how to create a minimal build script to compile a single-file "Hello, World!" program.

## Files in this example

- `main.c`: A simple C program that prints a message
- `build.c`: Build script that compiles the program using IncludeBuild

## Building the example

1. Compile the build script:
   ```bash
   gcc -o build build.c
   ```

2. Run the build script:
   ```bash
   ./build
   ```

This will compile `main.c` and create an executable called "basic".

## How it works

The build script:
1. Initializes IncludeBuild with `ib_init()`
2. Adds a target using `ib_add_target("basic", "main.c")`
3. Enables verbose output for detailed build information
4. Runs the build with `ib_build()`

This simple example demonstrates IncludeBuild's minimal API and how easy it is to set up a build script without any complex configuration files. The latest version of IncludeBuild automatically finds dependencies, rebuilds only when necessary, and intelligently excludes build script files (like build.c) from the compilation process. 