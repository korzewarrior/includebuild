# IncludeBuild Custom Limits Example

This example demonstrates how to customize the size limits in IncludeBuild to accommodate larger projects.

## Building and Running

Compile the build script:

```bash
gcc -o build build.c
```

Run the build script:

```bash
./build
```

## How It Works

By defining the limits before including `build.h`, you can override the defaults:

```c
// Define custom limits before including build
#define IB_MAX_FILES 2000      // Default is 1000
#define IB_MAX_DEPS 200        // Default is 100
#define IB_MAX_INCLUDE_DIRS 100 // Default is 50

#include "build.h"
```

This allows IncludeBuild to maintain its minimal and self-contained design while still being configurable for projects of different sizes.

## Available Limit Constants

You can customize any of these limits:

- `IB_MAX_PATH` - Maximum path length (default: 1024)
- `IB_MAX_CMD` - Maximum command length (default: 4096)
- `IB_MAX_FILES` - Maximum number of files (default: 1000)
- `IB_MAX_DEPS` - Maximum dependencies per file (default: 100)
- `IB_MAX_INCLUDE_DIRS` - Maximum include directories (default: 50)
- `IB_MAX_TARGETS` - Maximum number of targets (default: 50) 