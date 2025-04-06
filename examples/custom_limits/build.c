/**
 * IncludeBuild Custom Limits Example
 * 
 * This demonstrates how to customize IncludeBuild's internal limits
 * for larger projects by defining constants before including build.h.
 */

// Define custom limits before including build
#define IB_MAX_FILES 2000      // Default is 1000
#define IB_MAX_DEPS 200        // Default is 100
#define IB_MAX_INCLUDE_DIRS 100 // Default is 50

#include "../../build.h"
#include <stdio.h>

int main() {
    // Initialize IncludeBuild
    ib_init();
    
    // Enable verbose output to see what's happening
    ib_set_verbose(true);
    
    // Print out our custom limits
    printf("\nCustom limits in effect:\n");
    printf("  IB_MAX_FILES: %d (default: 1000)\n", IB_MAX_FILES);
    printf("  IB_MAX_DEPS: %d (default: 100)\n", IB_MAX_DEPS);
    printf("  IB_MAX_INCLUDE_DIRS: %d (default: 50)\n", IB_MAX_INCLUDE_DIRS);
    
    // Build the project - build.h will find main.c automatically
    ib_build();
    
    printf("\nBuild complete! Run './main' to execute the program.\n");
    
    return 0;
} 