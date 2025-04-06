/**
 * IncludeBuild Basic Example
 * 
 * This demonstrates how simple it is to build a basic single-file project.
 * Just include build.h, call ib_init() and ib_build() - that's it!
 */

#include "../../build.h"
#include <stdio.h>

int main() {
    // Initialize IncludeBuild
    ib_init();
    
    // Enable verbose output to see what's happening
    ib_set_verbose(true);
    
    // Build the project - build.h will find main.c automatically
    ib_build();
    
    printf("\nBuild complete! Run './main' to execute the program.\n");
    
    return 0;
} 