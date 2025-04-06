# IncludeBuild Multi-File Example

This example demonstrates how incredibly simple it is to build a C project with multiple source files using IncludeBuild.

## Magic Made Simple

IncludeBuild automatically finds and compiles all your source files with just two lines of code:

```c
ib_init();
ib_build();
```

No need to list files, set up complex build rules, or worry about dependencies. IncludeBuild handles it all!

## Files in this example:

- `math_utils.h` & `math_utils.c`: Math utility functions
- `string_utils.h` & `string_utils.c`: String utility functions
- `main.c`: Main program that uses both utilities
- `build.c`: Ultra-simple build script (just 3 effective lines!)

## Building the example

1. Compile the build script:
   ```bash
   gcc -o build build.c
   ```

2. Run the build script:
   ```bash
   ./build
   ```

3. Run the compiled program:
   ```bash
   ./main
   ```

## What IncludeBuild Does Automatically

- Finds all .c files in the project
- Detects which files include which headers
- Tracks dependencies between files
- Only rebuilds files that changed or depend on changes
- Links everything together correctly

## How it works

The entire build script is just a few lines of code:

```c
#include "../../build.h"

int main() {
    // Initialize IncludeBuild
    ib_init();
    
    // Build the project - build.h handles everything
    ib_build();
    
    return 0;
}
```

IncludeBuild does all the work for you! 