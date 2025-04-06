# IncludeBuild

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
![Version](https://img.shields.io/badge/version-1.0.0-green)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey)

**The header-only build system for C that just works.**

<p align="center">
  <img src="docs/images/banner.png" alt="IncludeBuild Banner" width="600">
</p>

## One Line to Build Them All

IncludeBuild turns build systems inside out: your build system is now **part of your code**.

```c
#include "build.h"  // That's it. This is your build system.

int main() {
    ib_init();
    ib_build();
    return 0;
}
```

## Philosophy

IncludeBuild embraces the philosophy that a build system should be:

1. **Minimalism**: Just one header file
2. **Transparency**: Your build system is plain C code
3. **Self-Contained**: No external dependencies
4. **Just Works**: Smart defaults with minimal configuration
5. **Part of the code**: Not separate from it

## How to Use

1. Copy `build.h` to your project
2. Create a build script:

```c
#include "build.h"

int main() {
    ib_init();
    ib_build();  // Automatically finds all source files
    return 0;
}
```

3. Compile and run your build script:

```bash
gcc -o build build.c
./build
```

That's it! IncludeBuild automatically:
- Finds all your source files
- Determines dependencies
- Compiles only what's changed
- Links everything correctly

## Advanced Features

While staying simple, IncludeBuild handles complex needs:

- **Multiple libraries**: `ib_add_libraries("lib1", "lib2", NULL)`
- **Static/dynamic libraries**: `ib_build_static_library()`
- **Custom flags**: Modify compiler and linker flags directly
- **Log levels**: Control build verbosity with `ib_set_log_level()`
- **Version checking**: Get version with `ib_version()`

## Examples

The `examples/` directory contains working examples:

- **basic**: A minimal "Hello, World!" program
- **multi_file**: Project with multiple source files
- **library**: Creating and using static/dynamic libraries
- **game**: Building a Raylib game (Pong clone)
- **logging**: Controlling build output verbosity
- **custom_limits**: Setting memory limits for larger projects

## Directory Structure

```
includebuild/
├── build.h           # The main header file - just copy this to your project
├── docs/             # Documentation files
├── examples/         # Example projects
├── scripts/          # Maintenance scripts
└── LICENSE           # MIT License
```

## Comparison with Other Build Systems

| Feature | IncludeBuild | Make | CMake |
|---------|------------|------|-------|
| Self-contained | ✓ Single header | × Requires Make | × Requires CMake |
| Learning curve | ✓ Just C | × Custom syntax | × Complex syntax |
| Power | ⚠️ Medium | ✓ High | ✓ Very high |
| Transparency | ✓ High | ⚠️ Medium | × Low |
| Large projects | ⚠️ Medium | ✓ Good | ✓ Excellent |

## Benefits

- **Simplicity**: Everything in C, no separate build language to learn
- **Transparency**: See exactly what's happening in your build process
- **Hackability**: Easy to modify and extend
- **Portability**: Just C code, works anywhere with a C compiler
- **Zero Setup**: No tools to install

## Documentation

- [User Guide](docs/guide.md)
- [API Reference](docs/api.md)
- [FAQ](docs/faq.md)

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

IncludeBuild is released under the MIT License. See [LICENSE](LICENSE) for details.

## Acknowledgments

IncludeBuild was inspired by the philosophy that build systems should be simple and transparent. It draws inspiration from projects like:

- [The Zig build system](https://ziglang.org/documentation/master/#Zig-Build-System)
- [Tup](http://gittup.org/tup/)
- [Meson](https://mesonbuild.com/)

## API Reference

### Core Functions

```c
// Initialize IncludeBuild with default settings
void ib_init(void);

// Initialize with custom configuration
void ib_init_with_config(const ib_config* config);

// Add an include directory
void ib_add_include_dir(const char* dir);

// Add a build target
void ib_add_target(const char* name, const char* main_source);

// Run the build process
bool ib_build(void);

// Clean build files
void ib_clean(void);

// Set logging level
void ib_set_log_level(ib_log_level level);
```

### Logging Levels

```c
typedef enum {
    IB_LOG_ERROR = 0,
    IB_LOG_WARN = 1,
    IB_LOG_INFO = 2,
    IB_LOG_DEBUG = 3
} ib_log_level;
```

### Advanced Configuration

IncludeBuild provides flexible configuration options:

```c
// Define custom limits for larger projects
#define IB_MAX_FILES 5000
#define IB_MAX_DEPS 500

#include "build.h"

int main() {
    // Initialize IncludeBuild
    ib_init();
    
    // Set logging level (ERROR, WARN, INFO, DEBUG)
    ib_set_log_level(IB_LOG_DEBUG);
    
    // Build the project
    ib_build();
    
    return 0;
}
```

You can define your own limits before including build:

```c
// Define custom limits for larger projects
#define IB_MAX_FILES 5000
#define IB_MAX_DEPS 500

#include "build.h"

int main() {
    // ... your build code ...
}
```

# IncludeBuild Logging Example

This example demonstrates how to use IncludeBuild's built-in logging system to control verbosity during the build process.

## What This Example Shows

IncludeBuild provides a flexible logging system that lets you control:

1. **Log Levels** - Filter messages by severity
2. **Colored Output** - Visual distinction between different types of messages
3. **Verbosity** - Control how much detail is shown

This example shows how to use all these features through command-line arguments.

## Directory Structure

```
logging/
├── build.c       - Build script demonstrating different log levels
├── main.c        - Simple application that gets built
└── README.md     - This file
```

## Building and Running

1. Compile the build script:
   ```bash
   gcc -o build build.c
   ```

2. Run the build script with different options to see how logging works:
   ```bash
   # Using default settings (INFO level, colored output)
   ./build
   
   # With most verbose logging
   ./build debug verbose
   
   # With errors only
   ./build error
   
   # Clean then build with warning level
   ./build clean warn
   
   # No colors, with warnings and errors only
   ./build color warn
   ```

3. Run the compiled demo program:
   ```bash
   ./logging_demo
   ```

## Command-Line Options

The build script accepts these options:

- `debug` - Set logging level to DEBUG (most verbose)
- `info` - Set logging level to INFO (default)
- `warn` - Set logging level to WARNING
- `error` - Set logging level to ERROR (errors only)
- `color` - Toggle colored output (on by default)
- `verbose` - Toggle verbose output (off by default)
- `clean` - Clean build artifacts before building
- `help` - Show help message

You can combine multiple options.

## IncludeBuild Log Levels

IncludeBuild supports these log levels (from most to least verbose):

- `IB_LOG_DEBUG` - All messages, including detailed internal operations
- `IB_LOG_INFO` - Normal informational messages (default)
- `IB_LOG_WARN` - Warnings and errors only
- `IB_LOG_ERROR` - Error messages only

## Colored Output

By default, IncludeBuild uses colored output to make it easier to distinguish between different types of messages:

- Red for errors
- Yellow for warnings
- Green for info messages
- Cyan for debug messages

## Verbose Output

For even more detailed information about the build process, enable verbose mode with the `verbose` option.

## Using These Features in Your Code

To use these features in your own IncludeBuild scripts:

```c
// Set the log level
ib_set_log_level(IB_LOG_DEBUG);  // Most verbose

// Control colored output
g_config.color_output = true;    // Enable colors

// Enable verbose mode
g_config.verbose = true;         // Show detailed output
```

## Learn More

For more information about IncludeBuild's logging features, check the main documentation. 