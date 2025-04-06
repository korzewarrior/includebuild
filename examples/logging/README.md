# IncludeBuild Logging Example

This example demonstrates how IncludeBuild provides powerful logging capabilities with minimal code.

## What This Example Shows

IncludeBuild includes a built-in logging system that lets you control message verbosity with just a single function call. This example shows how easy it is to:

1. Change log levels (DEBUG, INFO, WARNING, ERROR)
2. Use colored output automatically
3. Toggle verbose build information
4. Clean build artifacts

## Directory Structure

```
logging/
├── build.c       - Minimal build script with logging features
├── main.c        - Simple application that gets built
└── README.md     - This file
```

## Building and Running

1. Compile the build script:
   ```bash
   gcc -o build build.c
   ```

2. Run the build script with different log levels:
   ```bash
   ./build         # Default INFO level
   ./build debug   # Most verbose log level
   ./build warn    # Only warnings and errors
   ./build error   # Only errors
   ./build verbose # Toggle verbose mode
   ./build clean   # Clean and rebuild
   ```

3. Run the compiled program:
   ```bash
   ./main
   ```

## How It Works

The entire build script is just a few lines of code:

```c
#include "../../build.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    // Initialize IncludeBuild
    ib_init();
    
    // Enable verbose output to see what's happening
    g_config.verbose = true;
    
    // Set log level if specified
    if (argc > 1 && strcmp(argv[1], "debug") == 0)
        ib_set_log_level(IB_LOG_DEBUG);
    
    // Build the project - build.h handles everything
    ib_build();
    
    return 0;
}
```

IncludeBuild does all the work for you - finding source files, compiling them, and creating the executable.

## Log Levels

IncludeBuild supports these log levels:

- `IB_LOG_DEBUG` - All messages (most verbose)
- `IB_LOG_INFO` - Informational messages (default)
- `IB_LOG_WARN` - Only warnings and errors
- `IB_LOG_ERROR` - Only error messages

## Learn More

For more information about IncludeBuild's logging features, check the main documentation. 