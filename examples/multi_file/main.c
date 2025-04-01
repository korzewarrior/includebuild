#include "../../src/build.h"

int main() {
    // Initialize IncludeBuild with default settings
    ib_init();
    
    // Set the name of the executable to 'multi_file'
    ib_add_target("multi_file", "");
    
    // Enable verbose output to see what's happening
    g_config.verbose = true;
    
    // Set compiler flags
    strcpy(g_config.compiler_flags, "-Wall -Wextra -std=c99");
    
    // Build the project
    ib_build();
    
    printf("\nBuild complete. Run './build/multi_file' to execute the program.\n");
    printf("This example demonstrates how IncludeBuild handles multiple source files and dependencies.\n");
    printf("Try modifying one of the source files and rebuild - only the changed file will be recompiled!\n");
    
    return 0;
} 