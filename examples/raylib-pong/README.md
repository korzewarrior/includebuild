# Raylib Pong with IncludeBuild

This is an example of using IncludeBuild to compile a Raylib-based Pong game.

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

### First, compile the build script:

```bash
gcc -o build build.c
```

### Building for your current platform:

```bash
./build
```

### Building for Windows (cross-compilation from Linux):

```bash
./build windows
```

### Cleaning build files:

```bash
./build clean
```

## Playing the Game

- Use `W` and `S` keys to move the left paddle up and down
- Use `UP` and `DOWN` arrow keys to move the right paddle
- The ball speeds up slightly each time it hits a paddle
- Score is displayed at the top of the screen

## Understanding the Code

This example demonstrates how to use IncludeBuild with an external library (Raylib). The key components are:

1. **Raylib Integration**: The build script specifies the correct linker flags for Raylib
2. **Cross-Platform Building**: The code shows how to configure different platforms
3. **Command-Line Options**: Process command-line arguments to select build mode
4. **Dynamic Compilation**: Uses IncludeBuild to compile only what's necessary

## How IncludeBuild Helps

Traditional Makefile approach for a cross-platform Raylib game might look like:

```makefile
CC_LINUX = gcc
CC_WINDOWS = x86_64-w64-mingw32-gcc

CFLAGS = -Wall -Wextra -O2
CFLAGS_WINDOWS = $(CFLAGS) -I/usr/x86_64-w64-mingw32/include
LDFLAGS_LINUX = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
LDFLAGS_WINDOWS = -L/usr/x86_64-w64-mingw32/lib -lraylib -lopengl32 -lgdi32 -lwinmm

SRC = raypong.c
OUT_LINUX = raypong
OUT_WINDOWS = raypong.exe

all: linux

linux:
	$(CC_LINUX) $(CFLAGS) -o $(OUT_LINUX) $(SRC) $(LDFLAGS_LINUX)

windows:
	$(CC_WINDOWS) $(CFLAGS_WINDOWS) -o $(OUT_WINDOWS) $(SRC) $(LDFLAGS_WINDOWS)

clean:
	rm -f $(OUT_LINUX) $(OUT_WINDOWS)
```

IncludeBuild offers advantages:
1. Direct C programming interface
2. Built-in platform detection
3. Add logic for selecting options and configurations
4. Automatic dependency management

## Next Steps

After exploring this example, you can:
1. Add more game features (power-ups, sound effects)
2. Modify the build script to support more platforms
3. Add more command-line options
4. Create a build system for a bigger game project 