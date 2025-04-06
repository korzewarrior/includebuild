# IncludeBuild Game Example

This example demonstrates how to use IncludeBuild to compile a game that uses an external library (Raylib). 
It implements a simple Pong game to show how IncludeBuild handles external dependencies and cross-platform builds.

## What This Example Shows

This example demonstrates how IncludeBuild can:

1. Link with external libraries (Raylib)
2. Handle cross-platform configurations
3. Process command-line options for different build targets
4. Handle embedded binary data in header files

## Prerequisites

- GCC compiler (for Linux build)
- MinGW-w64 (for Windows cross-compilation)
- Raylib development libraries

### Installing Raylib

#### On Ubuntu/Debian:
```bash
sudo apt install libraylib-dev
```

#### On Fedora:
```bash
sudo dnf install raylib-devel
```

#### On Arch Linux:
```bash
sudo pacman -S raylib
```

#### On macOS (using Homebrew):
```bash
brew install raylib
```

#### On Windows:
Download from [raylib.com](https://www.raylib.com/) or use MSYS2 or vcpkg.

## Building the Game

1. Compile the build script:
   ```bash
   gcc -o build build.c
   ```

2. Build for your current platform:
   ```bash
   ./build
   ```

3. Build for Windows (cross-compilation from Linux):
   ```bash
   ./build windows
   ```

4. Clean build files:
   ```bash
   ./build clean
   ```

5. Show help:
   ```bash
   ./build help
   ```

## Playing the Game

- Use `UP` and `DOWN` keys to move your paddle (on the left)
- The AI controls the paddle on the right
- The ball speeds up slightly each time it hits a paddle
- Lives are displayed at the top of the screen

## How It Works

The build script demonstrates a more complex use case for IncludeBuild with:

```c
#include "../../build.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    // Initialize IncludeBuild
    ib_init();
    
    // Process command-line arguments
    bool windows_build = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "windows") == 0)
            windows_build = true;
    }
    
    // Set platform-specific configurations
    if (windows_build) {
        strcpy(g_config.compiler, "x86_64-w64-mingw32-gcc");
        strcpy(g_config.linker_flags, "-lraylib -lopengl32 -lgdi32 -lwinmm");
        ib_add_target("main.exe", "main.c");
    } else {
        strcpy(g_config.linker_flags, "-lraylib -lGL -lm -lpthread -ldl -lrt -lX11");
        ib_add_target("main", "main.c");
    }
    
    // Build the project
    ib_build();
    
    return 0;
}
```

### Embedding Assets

This game embeds all its assets (sounds, music) directly into header files, making distribution simpler. For example:

```c
#ifndef PADDLE_HIT_H
#define PADDLE_HIT_H

const unsigned int paddle_hit_wav_len = /* data length */;
const unsigned char paddle_hit_wav[] = {
    /* binary data for sound */
};

#endif
```

IncludeBuild offers several advantages for game development:
1. Direct C programming interface
2. Built-in platform detection
3. Logical control flow for selecting options and configurations
4. Automatic dependency management
5. Simple integration with external libraries

## Next Steps

After exploring this example, you can:
1. Add more game features (power-ups, multiplayer)
2. Modify the build script to support more platforms
3. Add more command-line options
4. Create a build system for a bigger game project 