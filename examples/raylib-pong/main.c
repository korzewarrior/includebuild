/**
 * Build script for Raylib Pong using IncludeBuild
 */
#include "../../src/build.h"
#include <stdio.h>
#include <string.h>

void print_usage(const char* program_name) {
    printf("Usage: %s [command] [platform]\n\n", program_name);
    printf("Commands:\n");
    printf("  build    Build the project (default if no command specified)\n");
    printf("  clean    Clean the build directory\n");
    printf("  help     Show this help message\n");
    printf("\nPlatforms:\n");
    printf("  linux    Build for Linux (default)\n");
    printf("  windows  Build for Windows using MinGW\n");
}

int main(int argc, char** argv) {
    // Process command line arguments
    bool clean_build = false;
    bool windows_build = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "clean") == 0) {
            clean_build = true;
        } else if (strcmp(argv[i], "windows") == 0) {
            windows_build = true;
        }
    }
    
    // Initialize IncludeBuild
    ib_init();
    
    // Enable verbose output
    g_config.verbose = true;
    
    // Set compiler flags
    #ifdef _WIN32
    strcpy(g_config.compiler_flags, "/nologo /W3 /O2");
    #else
    strcpy(g_config.compiler_flags, "-Wall -Wextra -O2");
    #endif
    
    // Set linker flags for Raylib
    #ifdef _WIN32
    // Windows-specific linker flags
    strcpy(g_config.linker_flags, "-lraylib -lopengl32 -lgdi32 -lwinmm");
    #else
    // Linux/macOS linker flags
    strcpy(g_config.linker_flags, "-lraylib -lGL -lm -lpthread -ldl -lrt -lX11");
    #endif
    
    // Clean if requested
    if (clean_build) {
        printf("Cleaning...\n");
        ib_clean();
        return 0;
    }
    
    // Special case for cross-compiling to Windows from Linux
    #if !defined(_WIN32)
    if (windows_build) {
        printf("Cross-compiling for Windows...\n");
        
        // Use MinGW compiler
        strcpy(g_config.compiler, "x86_64-w64-mingw32-gcc");
        
        // Add Windows include paths
        ib_add_include_dir("/usr/x86_64-w64-mingw32/include");
        
        // Set Windows target
        ib_add_target("raypong.exe", "raypong.c");
        
        // Windows linker flags
        strcpy(g_config.linker_flags, "-lraylib -lopengl32 -lgdi32 -lwinmm");
    } else {
    #endif
        // Regular build for current platform
        ib_add_target("raypong", "raypong.c");
    #if !defined(_WIN32)
    }
    #endif
    
    // Build the project
    printf("Building Raylib Pong game...\n");
    ib_build();
    
    printf("\nBuild complete!\n");
    return 0;
} 