# IncludeBuild API Reference

This document provides a complete reference for the IncludeBuild API.

## Workspace Management

### ib_workspace_t

The primary structure that holds the build configuration and state.

```c
typedef struct ib_workspace_t* ib_workspace_t;
```

### ib_create_workspace()

Creates a new workspace with default configuration.

```c
ib_workspace_t ib_create_workspace(void);
```

### ib_destroy_workspace()

Cleans up and releases a workspace when done.

```c
void ib_destroy_workspace(ib_workspace_t workspace);
```

## Source Files

### ib_add_source_file()

Adds a C/C++ source file to the build.

```c
bool ib_add_source_file(ib_workspace_t workspace, const char* file_path);
```

### ib_add_source_directory()

Adds all source files in a directory.

```c
int ib_add_source_directory(ib_workspace_t workspace, const char* directory_path, bool recursive);
```

## Include Directories

### ib_add_include_dir()

Adds an include directory for the compiler to search.

```c
bool ib_add_include_dir(ib_workspace_t workspace, const char* dir_path);
```

## Libraries and Linking

### ib_add_library()

Adds a library to link against.

```c
bool ib_add_library(ib_workspace_t workspace, const char* lib_name);
```

### ib_add_library_dir()

Adds a directory path for the linker to search for libraries.

```c
bool ib_add_library_dir(ib_workspace_t workspace, const char* dir_path);
```

## Compiler and Linker Flags

### ib_add_compiler_flag()

Adds a compiler flag.

```c
bool ib_add_compiler_flag(ib_workspace_t workspace, const char* flag);
```

### ib_add_linker_flag()

Adds a linker flag.

```c
bool ib_add_linker_flag(ib_workspace_t workspace, const char* flag);
```

## Output Configuration

### ib_set_output()

Sets the name of the output executable.

```c
bool ib_set_output(ib_workspace_t workspace, const char* output_name);
```

### ib_set_build_dir()

Sets the directory for build artifacts.

```c
bool ib_set_build_dir(ib_workspace_t workspace, const char* build_dir);
```

## Build Execution

### ib_build()

Executes the build according to the configuration.

```c
bool ib_build(ib_workspace_t workspace);
```

### ib_clean()

Cleans build artifacts.

```c
bool ib_clean(ib_workspace_t workspace);
```

## Error Handling

### ib_get_last_error()

Gets the most recent error message.

```c
const char* ib_get_last_error(ib_workspace_t workspace);
```

## Logging and Diagnostics

### ib_set_log_level()

Sets the verbosity of logging.

```c
void ib_set_log_level(ib_workspace_t workspace, ib_log_level_t level);
```

### ib_set_log_callback()

Sets a custom callback for log messages.

```c
void ib_set_log_callback(ib_workspace_t workspace, ib_log_callback_t callback, void* user_data);
```

## Custom Compilation

### ib_set_compiler_executable()

Overrides the default compiler.

```c
bool ib_set_compiler_executable(ib_workspace_t workspace, const char* compiler_path);
```

### ib_set_linker_executable()

Overrides the default linker.

```c
bool ib_set_linker_executable(ib_workspace_t workspace, const char* linker_path);
```

## Configurable Limits

You can override these in your own code before including the header:

```c
#define IB_MAX_SOURCE_FILES 1000  // Default is 500
#define IB_MAX_INCLUDE_DIRS 100   // Default is 50
#define IB_MAX_LIBRARY_DIRS 50    // Default is 20
#include "build.h"
```

## Full Example

```c
#include "build.h"

int main() {
    ib_workspace_t ws = ib_create_workspace();
    
    // Configure the build
    ib_set_output(ws, "my_program");
    ib_set_build_dir(ws, "build");
    
    // Add source files
    ib_add_source_file(ws, "src/main.c");
    ib_add_source_directory(ws, "src/modules", true);
    
    // Add include paths
    ib_add_include_dir(ws, "include");
    
    // Add libraries
    ib_add_library(ws, "m");  // libm (math library)
    
    // Add compiler flags
    ib_add_compiler_flag(ws, "-Wall");
    ib_add_compiler_flag(ws, "-O2");
    
    // Run the build
    if (!ib_build(ws)) {
        printf("Build failed: %s\n", ib_get_last_error(ws));
        ib_destroy_workspace(ws);
        return 1;
    }
    
    // Clean up
    ib_destroy_workspace(ws);
    return 0;
}
``` 