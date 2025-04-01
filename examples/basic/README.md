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
1. Initializes IncludeBuild
2. Adds the main.c file as a target
3. Runs the build

This simple example demonstrates IncludeBuild's minimal API and how easy it is to set up a build script without any complex configuration files. 