// Define custom limits before including build
#define IB_MAX_FILES 2000      // Default is 1000
#define IB_MAX_DEPS 200        // Default is 100
#define IB_MAX_INCLUDE_DIRS 100 // Default is 50

#include "../../src/build.h"
#include <stdio.h>

int main() {
    // Initialize IncludeBuild with default settings
    ib_init();
    
    // Set the target name
    ib_add_target("custom_limits_demo", "main.c");
    
    // Print out our custom limits
    printf("\nCustom limits in effect:\n");
    printf("  IB_MAX_FILES: %d (default: 1000)\n", IB_MAX_FILES);
    printf("  IB_MAX_DEPS: %d (default: 100)\n", IB_MAX_DEPS);
    printf("  IB_MAX_INCLUDE_DIRS: %d (default: 50)\n", IB_MAX_INCLUDE_DIRS);
    
    // Build the project
    ib_build();
    
    printf("\nBuild complete. Run './build/custom_limits_demo' to execute the program.\n");
    
    return 0;
} 