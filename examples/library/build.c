#include "../../src/build.h"
#include <string.h>

int main(int argc, char** argv) {
    // Initialize IncludeBuild
    ib_init();
    
    // Set configuration
    g_config.verbose = true;
    
    // Add include directory
    ib_add_include_dir("include");
    
    // Parse command line arguments
    bool build_static = true;
    bool build_dynamic = true;
    bool build_test = true;
    
    if (argc > 1) {
        if (strcmp(argv[1], "clean") == 0) {
            ib_clean();
            return 0;
        } else if (strcmp(argv[1], "static") == 0) {
            build_dynamic = false;
            build_test = false;
        } else if (strcmp(argv[1], "dynamic") == 0) {
            build_static = false;
            build_test = false;
        } else if (strcmp(argv[1], "test") == 0) {
            build_static = false;
            build_dynamic = false;
        } else {
            printf("Unknown command: %s\n", argv[1]);
            printf("Usage: %s [clean|static|dynamic|test]\n", argv[0]);
            return 1;
        }
    }
    
    // Build static library
    if (build_static) {
        printf("\n--- Building static library ---\n");
        
        // Add target for static library
        ib_add_target("liblogger", "src/logger.c");
        
        // Mark it as a library
        g_targets[g_num_targets-1].is_library = true;
        
        // Use ar command for static library creation
        strcpy(g_config.compiler_flags, "-c -fPIC");
        
        // Build the static library
        ib_build();
        
        // Manually create static library from object files
        #ifdef _WIN32
        system("mkdir -p lib && ar rcs lib/liblogger.lib build/*.obj");
        #else
        system("mkdir -p lib && ar rcs lib/liblogger.a build/*.o");
        #endif
    }
    
    // Build dynamic library
    if (build_dynamic) {
        printf("\n--- Building dynamic library ---\n");
        
        // Reset configuration
        ib_init();
        g_config.verbose = true;
        ib_add_include_dir("include");
        
        // Add target for dynamic library
        ib_add_target("liblogger", "src/logger.c");
        
        // Mark it as a library
        g_targets[g_num_targets-1].is_library = true;
        
        // Use shared library flags
        strcpy(g_config.compiler_flags, "-c -fPIC");
        
        // Build the dynamic library
        ib_build();
        
        // Manually create dynamic library from object files
        #ifdef _WIN32
        system("mkdir -p lib && gcc -shared -o lib/liblogger.dll build/*.obj");
        #else
        system("mkdir -p lib && gcc -shared -o lib/liblogger.so build/*.o");
        #endif
    }
    
    // Build test executable
    if (build_test) {
        printf("\n--- Building test executable ---\n");
        
        // Reset config
        ib_init();
        g_config.verbose = true;
        ib_add_include_dir("include");
        
        // Set linker flags to link with the static library
        #ifdef _WIN32
        strcpy(g_config.linker_flags, "lib/liblogger.lib");
        #else
        strcpy(g_config.linker_flags, "-Llib -llogger");
        #endif
        
        // Add target
        ib_add_target("logger_test", "test_logger.c");
        
        // Build the test executable
        ib_build();
        
        // Copy the executable to current directory
        system("cp build/logger_test .");
    }
    
    printf("\nBuild complete.\n");
    
    if (build_test) {
        printf("\nRun './logger_test' to test the logger library.\n");
    }
    
    return 0;
} 