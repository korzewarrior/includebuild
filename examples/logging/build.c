/**
 * IncludeBuild Logging Example
 * 
 * This demonstrates how simple it is to use IncludeBuild's logging features.
 * You can control logging verbosity with minimal code.
 */

#include "../../build.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    // Initialize IncludeBuild
    ib_init();
    
    // Enable verbose output to see what's happening
    ib_set_verbose(true);
    
    // Set a different log level if specified by command line
    if (argc > 1) {
        if (strcmp(argv[1], "debug") == 0)
            ib_set_log_level(IB_LOG_DEBUG);      // Most verbose
        else if (strcmp(argv[1], "warn") == 0)
            ib_set_log_level(IB_LOG_WARN);       // Warnings and errors
        else if (strcmp(argv[1], "error") == 0)
            ib_set_log_level(IB_LOG_ERROR);      // Errors only
        else if (strcmp(argv[1], "clean") == 0)
            ib_clean();                          // Clean build artifacts
        else if (strcmp(argv[1], "verbose") == 0)
            ib_set_verbose(false);               // Toggle verbose mode (assumed true above)
    }
    
    // Build the project - build.h will find main.c automatically
    ib_build();
    
    printf("\nBuild complete! Run './main' to execute the program.\n");
    return 0;
}
