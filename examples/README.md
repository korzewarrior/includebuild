# IncludeBuild Examples

This directory contains several example projects that demonstrate how to use IncludeBuild in different scenarios. Each example is self-contained and can be built by navigating to its directory and running the build script.

## Example Projects

### 1. Basic Example

**Location:** [`basic/`](basic/)

A minimal example that demonstrates the simplest possible use of IncludeBuild. It builds a single "Hello, World!" program.

**To build:**
```bash
cd basic
gcc -o build build.c
./build
```

**Key concepts demonstrated:**
- Minimal build script setup
- Default build configuration
- Automatic source file discovery

### 2. Multi-File Example

**Location:** [`multi_file/`](multi_file/)

This example shows how IncludeBuild handles a project with multiple source files and demonstrates automatic dependency resolution.

**To build:**
```bash
cd multi_file
gcc -o build build.c
./build
```

**Key concepts demonstrated:**
- Multi-file project organization
- Header dependencies
- Rebuilding only what's necessary when files change

### 3. Library Example

**Location:** [`library/`](library/)

Shows how to create and use static and dynamic libraries with IncludeBuild.

**To build:**
```bash
cd library
gcc -o build build.c
./build
```

**Key concepts demonstrated:**
- Creating static libraries
- Creating dynamic libraries
- Using libraries in executables

### 4. Raylib Pong Example

**Location:** [`raylib_pong/`](raylib_pong/)

A more complex example showing how to build a simple Pong game using the Raylib library. This demonstrates how IncludeBuild can handle external dependencies.

**Prerequisites:**
- Raylib must be installed on your system
- On Linux: `sudo apt install libraylib-dev` (or equivalent for your distribution)
- On Windows: Install Raylib or provide the library files in the `lib` directory

**To build:**
```bash
cd raylib_pong
gcc -o build build.c
./build
```

**Key concepts demonstrated:**
- Linking with external libraries
- Platform-specific compilation
- More complex project structure
- Custom build configurations

### 5. Logging Example

**Location:** [`logging/`](logging/)

Demonstrates the enhanced logging features of IncludeBuild, including different log levels.

**To build:**
```bash
cd logging
gcc -o build build.c
./build
```

**Key concepts demonstrated:**
- Log levels (ERROR, WARN, INFO, DEBUG)
- Setting and changing log levels
- How logging affects output verbosity
- Improved error messages

### 6. Custom Limits Example

**Location:** [`custom_limits/`](custom_limits/)

Shows how to customize the build system limits to support larger projects.

**To build:**
```bash
cd custom_limits
gcc -o build build.c
./build
```

**Key concepts demonstrated:**
- Configuring limits before including build
- Overriding default size constraints
- Maintaining minimalism while supporting scalability

## Running the Examples

Each example directory contains a `build.c` file that serves as the build script for IncludeBuild. To run an example:

1. Navigate to the example directory: `cd examples/basic`
2. Compile the build script: `gcc -o build build.c`
3. Run the build: `./build`
4. Run the resulting executable: `./basic` (or the name of the target)

## Modifying the Examples

Feel free to modify the examples to experiment with IncludeBuild features. Here are some suggestions:

- Add more source files to see how the dependency system works
- Change compiler flags to see how they affect the build
- Add command-line options to the build scripts
- Try building for different platforms

## Cross-Platform Support

All examples work on Linux, macOS, and Windows with appropriate compilers installed. The build scripts include platform-specific code where necessary.

## Contributing New Examples

If you've created a useful example that demonstrates IncludeBuild features, consider contributing it back to the project. See the [CONTRIBUTING.md](../CONTRIBUTING.md) file for guidelines.

## Additional Resources

For more information on using IncludeBuild, refer to:

- [User Guide](../docs/guide.md) - Comprehensive guide to using IncludeBuild
- [API Reference](../docs/api.md) - Detailed documentation of all IncludeBuild functions
- [FAQ](../docs/faq.md) - Answers to frequently asked questions 