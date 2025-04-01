#include "../../src/build.h"

int main() {
    // Initialize IncludeBuild with default settings
    ib_init();
    
    // Set the target name to avoid conflict with this build script
    ib_add_target("logging_demo", "app.c");
    
    // Enable verbose output 
    g_config.verbose = true;
    
    // Set log level to debug (most verbose)
    ib_set_log_level(IB_LOG_DEBUG);
    
    printf("\nBuilding with DEBUG log level...\n");
    
    // Build the project
    ib_build();
    
    // Now change to less verbose
    ib_set_log_level(IB_LOG_ERROR);
    printf("\nChanging to ERROR-only log level...\n");
    
    // Clean and rebuild
    ib_clean();
    ib_build();
    
    printf("\nBuild complete. Run './build/logging_demo' to execute the program.\n");
    
    return 0;
} 