#include "../../src/build.h"

int main() {
    // Initialize IncludeBuild with default settings
    ib_init();
    
    // Set the name of the executable to 'basic'
    ib_add_target("basic", "main.c");
    
    // Enable verbose output to see what's happening
    g_config.verbose = true;
    
    // Build the project
    ib_build();
    
    printf("\nBuild complete. Run './build/basic' to execute the program.\n");
    
    return 0;
} 