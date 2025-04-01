# IncludeBuild Library Example

This example demonstrates how to use IncludeBuild to create and use static and dynamic libraries. It includes a simple logging library with different log levels and output options.

## Project Structure

```
library/
├── include/          # Public header files
│   └── logger.h      # Public API for the logger
├── src/              # Implementation files
│   └── logger.c      # Logger implementation
├── test_logger.c     # Test program that uses the library
├── build.c           # IncludeBuild script
└── README.md         # This file
```

## Building the Example

Compile the build script:

```bash
gcc -o build build.c
```

Run the build script:

```bash
./build             # Build everything (static lib, dynamic lib, and test program)
./build static      # Build only the static library
./build dynamic     # Build only the dynamic library
./build test        # Build only the test program
./build clean       # Clean build files
```

This will:
1. Create a static library (`lib/liblogger.a`)
2. Create a dynamic library (`lib/liblogger.so` or `lib/liblogger.dll`)
3. Build a test program that uses the library

## Using the Library

The library provides a simple logging API with different log levels:

```c
#include "logger.h"

int main() {
    // Initialize the logger
    logger_init(LOG_DEBUG);  // Show all messages
    
    // Log messages
    log_error("This is an error message");
    log_warn("This is a warning message");
    log_info("This is an info message");
    log_debug("This is a debug message");
    
    // Cleanup
    logger_shutdown();
    
    return 0;
}
```

## How It Works

The build script demonstrates several key concepts:

1. **Building Static Libraries**: Using `ar` to create a static library from object files
2. **Building Dynamic Libraries**: Using `-shared` to create a dynamic library
3. **Include Directories**: Adding include directories with `ib_add_include_dir()`
4. **Linking**: Setting linker flags to link against the library
5. **Command-Line Arguments**: Processing arguments to control what gets built

## Next Steps

This example showcases several important IncludeBuild features:
1. Building multiple target types (executables and libraries)
2. Configuring include paths
3. Setting compiler and linker flags
4. Processing command-line arguments
5. Building in stages (compile → link → post-process)

Feel free to modify this example to experiment with IncludeBuild. Here are some ideas:
1. Add more functions to the library
2. Create a more complex directory structure
3. Add library versioning
4. Create headers with inline functions

This is a great starting point to understand how to use IncludeBuild for more complex projects with library dependencies. 