# IncludeBuild Logging Example

This example demonstrates the enhanced logging and configurable limits features of IncludeBuild.

## Building and Running

Compile the build script:

```bash
gcc -o build build.c
```

Run the build script:

```bash
./build
```

This will:
1. Build the project with DEBUG level logging (most verbose)
2. Clean the project
3. Rebuild with ERROR-only logging (least verbose)

## Configurable Limits

IncludeBuild allows you to customize the limits used in the build system by defining them before including the header:

```c
// Example: Custom limits for a larger project
#define IB_MAX_FILES 5000  // Default is 1000
#define IB_MAX_DEPS 500    // Default is 100
#include "build.h"
```

## Log Levels

IncludeBuild supports the following log levels:

- `IB_LOG_ERROR` - Only show errors (most critical)
- `IB_LOG_WARN` - Show errors and warnings
- `IB_LOG_INFO` - Show normal informational messages (default)
- `IB_LOG_DEBUG` - Show detailed debug information (most verbose)

You can set the log level using:

```c
ib_set_log_level(IB_LOG_DEBUG);
``` 