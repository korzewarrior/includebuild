/**
 * IncludeBuild Multi-File Example
 * 
 * This demonstrates how easy it is to build projects with multiple source files.
 * IncludeBuild automatically handles dependencies between files.
 */

#include "../../build.h"
#include <stdio.h>

int main() {
    // Initialize IncludeBuild
    ib_init();
    
    // Enable verbose output to see what's happening
    ib_set_verbose(true);
    
    // Build the project - build.h will find all source files automatically
    ib_build();
    
    printf("\nBuild complete! Run './main' to execute the program.\n");
    return 0;
} 