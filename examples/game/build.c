/**
 * IncludeBuild Game Example
 * 
 * This demonstrates how to use IncludeBuild to compile a game
 * that uses the Raylib external library.
 */

#include "../../build.h"
#include <stdio.h>

int main() {
    // Initialize IncludeBuild
    ib_init();
    
    // Enable verbose output to see what's happening
    ib_set_verbose(true);
    
    // Add Raylib and its dependencies
    ib_add_libraries("raylib", "GL", "m", "pthread", "dl", "rt", "X11", NULL);
    
    // Build the project - build.h will find main.c automatically
    ib_build();
    
    printf("\nBuild complete! Run './main' to play the game.\n");
    
    return 0;
} 