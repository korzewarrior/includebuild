/**
 * MIT License
 *
 * Copyright (c) 2025 IncludeBuild Contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * IncludeBuild - A minimalist build system for C/C++ projects
 * 
 * Just #include "build.h" and you're ready to go
 * 
 * Basic usage:
 *   #include "build.h"
 *   
 *   int main() {
 *       ib_init();
 *       ib_build();
 *       return 0;
 *   }
 */

#ifndef INCLUDEBUILD_H
#define INCLUDEBUILD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEPARATOR '\\'
#define popen _popen
#define pclose _pclose
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/wait.h>
#define PATH_SEPARATOR '/'
#endif

// Version information
#define IB_VERSION_MAJOR 1
#define IB_VERSION_MINOR 0
#define IB_VERSION_PATCH 0

// Default limits (can be overridden at compile time)
#ifndef IB_MAX_PATH
#define IB_MAX_PATH 1024
#endif

#ifndef IB_MAX_CMD
#define IB_MAX_CMD 4096
#endif

#ifndef IB_MAX_FILES
#define IB_MAX_FILES 1000
#endif

#ifndef IB_MAX_DEPS
#define IB_MAX_DEPS 100
#endif

#ifndef IB_MAX_INCLUDE_DIRS
#define IB_MAX_INCLUDE_DIRS 50
#endif

#ifndef IB_MAX_TARGETS
#define IB_MAX_TARGETS 50
#endif

#ifndef IB_MAX_LIBRARIES
#define IB_MAX_LIBRARIES 50
#endif

#ifndef IB_MAX_LIBRARY_PATHS
#define IB_MAX_LIBRARY_PATHS 50
#endif

// Whether to run the executable after building
static bool g_run_after_build = false;
static char g_executable_name[IB_MAX_PATH] = {0};

// Logging levels
typedef enum {
    IB_LOG_ERROR = 0,
    IB_LOG_WARN = 1,
    IB_LOG_INFO = 2,
    IB_LOG_DEBUG = 3
} ib_log_level;

// Forward declarations
typedef struct ib_file ib_file;
typedef struct ib_target ib_target;
typedef struct ib_config ib_config;

struct ib_file {
    char path[IB_MAX_PATH];             // Path to file
    char obj_path[IB_MAX_PATH];         // Path to output object file
    time_t last_modified;               // Last modified timestamp
    int num_deps;                       // Number of dependencies
    ib_file* deps[IB_MAX_DEPS];         // Dependencies
    bool needs_rebuild;                 // Whether file needs rebuilding
};

struct ib_target {
    char name[IB_MAX_PATH];             // Target name
    char output_path[IB_MAX_PATH];      // Output path
    char main_source[IB_MAX_PATH];      // Main source file
    int num_files;                      // Number of files in this target
    ib_file* files[IB_MAX_FILES];       // Files that make up this target
    bool is_library;                    // Whether this is a library
};

struct ib_config {
    char source_dir[IB_MAX_PATH];       // Source directory
    char build_dir[IB_MAX_PATH];        // Build directory for final executables
    char obj_dir[IB_MAX_PATH];          // Directory for object files
    char compiler[64];                  // Compiler to use
    char compiler_flags[IB_MAX_CMD];    // Compiler flags
    char linker_flags[IB_MAX_CMD];      // Linker flags
    char include_dirs[IB_MAX_INCLUDE_DIRS][IB_MAX_PATH]; // Include directories
    int num_include_dirs;               // Number of include directories
    char exclude_files[IB_MAX_FILES][IB_MAX_PATH]; // Files to exclude from build
    int num_exclude_files;              // Number of excluded files
    char libraries[IB_MAX_LIBRARIES][64];  // Libraries to link with
    int num_libraries;                  // Number of libraries
    char library_paths[IB_MAX_LIBRARY_PATHS][IB_MAX_PATH]; // Library paths
    int num_library_paths;              // Number of library paths
    bool verbose;                       // Verbose output
    bool color_output;                  // Colorize output
    ib_log_level log_level;             // Current logging level
};

// Global state
static ib_config g_config;
static ib_file g_files[IB_MAX_FILES];
static int g_num_files = 0;
static ib_target g_targets[IB_MAX_TARGETS];
static int g_num_targets = 0;
static bool g_initialized = false;

// ANSI color codes
#define IB_COLOR_RESET   "\x1b[0m"
#define IB_COLOR_RED     "\x1b[31m"
#define IB_COLOR_GREEN   "\x1b[32m"
#define IB_COLOR_YELLOW  "\x1b[33m"
#define IB_COLOR_BLUE    "\x1b[34m"
#define IB_COLOR_MAGENTA "\x1b[35m"
#define IB_COLOR_CYAN    "\x1b[36m"

// Helper macros for colorized output - using a simpler version to avoid line break issues
#define IB_COLOR(color, text) ((g_config.color_output) ? (color text IB_COLOR_RESET) : (text))

// Function declarations
static void ib_log_message(ib_log_level level, const char* fmt, ...);
static void ib_error(const char* fmt, ...);
static void ib_warning(const char* fmt, ...);
static bool ib_file_exists(const char* path);
static time_t ib_get_file_mtime(const char* path);
static void ib_ensure_dir_exists(const char* path);
static char* ib_join_path(char* dest, const char* path1, const char* path2);
static void ib_find_source_files(const char* dir);
static void ib_parse_dependencies(ib_file* file);
static bool ib_needs_rebuild(ib_file* file);
static void ib_compile_file(ib_file* file);
static void ib_link_target(ib_target* target);
static void ib_add_default_target(void);
static void ib_reset_targets(void);
static void ib_reset_files(void);
static bool ib_execute_command(const char* cmd);
static void ib_create_run_script(const char* program_name, const char* lib_dir);

// Public function forward declarations
void ib_init(void);
void ib_clean(void);
void ib_add_include_dir(const char* dir);
void ib_add_target(const char* name, const char* main_source);
bool ib_build(void);
void ib_reset_config(void);
void ib_add_library(const char* library);
void ib_add_libraries(const char* first, ...);
void ib_add_library_path(const char* path);
void ib_exclude_file(const char* file);
bool ib_build_static_library(const char* name, const char* main_source, const char* exclude_file);
bool ib_build_dynamic_library(const char* name, const char* main_source, const char* exclude_file);
const char* ib_version(void);

/**
 * Set whether to run the executable after building
 * @param run_after_build Whether to run the executable after build
 * @param executable_name Name of the executable to run (if NULL, will run first target)
 */
void ib_set_run_after_build(bool run_after_build, const char* executable_name) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return;
    }
    
    g_run_after_build = run_after_build;
    
    if (executable_name && executable_name[0] != '\0') {
        strncpy(g_executable_name, executable_name, IB_MAX_PATH - 1);
        g_executable_name[IB_MAX_PATH - 1] = '\0';
    } else {
        g_executable_name[0] = '\0'; // Use default target
    }
    
    ib_log_message(IB_LOG_INFO, "Automatic run after build: %s", run_after_build ? "enabled" : "disabled");
}

/**
 * Run an executable with the appropriate environment variables
 * @param executable_name Name of the executable to run
 * @return true if successful, false otherwise
 */
bool ib_run_executable(const char* executable_name) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return false;
    }
    
    if (!executable_name || executable_name[0] == '\0') {
        ib_error("No executable name specified");
        return false;
    }
    
    if (!ib_file_exists(executable_name)) {
        ib_error("Executable not found: %s", executable_name);
        return false;
    }
    
    ib_log_message(IB_LOG_INFO, "Running executable: %s", executable_name);
    
    // Construct the command to run the executable
    // On Unix systems, include the current directory in the library path
    char cmd[IB_MAX_CMD];
    #ifdef _WIN32
    snprintf(cmd, IB_MAX_CMD, "%s", executable_name);
    #else
    snprintf(cmd, IB_MAX_CMD, "LD_LIBRARY_PATH=\"$(pwd)/lib:$LD_LIBRARY_PATH\" ./%s", executable_name);
    #endif
    
    return ib_execute_command(cmd);
}

/**
 * Automatically build a complete library with minimal configuration
 * 
 * This is the "magic" function that handles everything:
 * - Finds source files in src/ directory
 * - Creates static library
 * - Creates dynamic/shared library
 * - Finds and builds test programs
 * - Creates helper scripts for testing
 * - Processes command-line arguments
 * 
 * @param library_name Base name of the library (e.g., "logger" creates "liblogger.a/so")
 * @param argc Number of command line arguments (from main)
 * @param argv Command line arguments (from main)
 * @return true if successful
 */
bool ib_build_library(const char* library_name, int argc, char** argv) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return false;
    }
    
    // Create directory names
    char lib_dir[IB_MAX_PATH] = "lib";
    char src_dir[IB_MAX_PATH] = "src";
    
    // Default to building everything
    bool build_static = true;
    bool build_dynamic = true; 
    bool build_test = true;
    bool do_clean = false;
    
    // Process command-line arguments if provided
    if (argc > 1) {
        if (strcmp(argv[1], "clean") == 0) {
            do_clean = true;
            build_static = false;
            build_dynamic = false;
            build_test = false;
        } else if (strcmp(argv[1], "static") == 0) {
            build_dynamic = false;
            build_test = false;
        } else if (strcmp(argv[1], "dynamic") == 0) {
            build_static = false;
            build_test = false;
        } else if (strcmp(argv[1], "test") == 0) {
            build_static = false;
            build_dynamic = false;
        } else if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("\nIncludeBuild Library Builder\n");
            printf("==========================\n\n");
            printf("Usage: %s [command]\n\n", argv[0]);
            printf("Commands:\n");
            printf("  (no args) - Build everything\n");
            printf("  static    - Build only the static library\n");
            printf("  dynamic   - Build only the dynamic library\n");
            printf("  test      - Build only the test program(s)\n");
            printf("  clean     - Remove all build artifacts\n");
            printf("  help      - Show this help message\n");
            return true;
        }
    }
    
    // Handle cleaning if requested
    if (do_clean) {
        ib_log_message(IB_LOG_INFO, "Cleaning build artifacts");
        ib_clean();
        
        // Also clean library files and test programs
        char cmd[IB_MAX_CMD] = {0};
        sprintf(cmd, "rm -rf %s run_*.sh test_* %s_test*", lib_dir, library_name);
        system(cmd);
        
        ib_log_message(IB_LOG_INFO, "All build artifacts removed");
        return true;
    }
    
    // Create lib directory if it doesn't exist and we're building libraries
    if ((build_static || build_dynamic) && !ib_file_exists(lib_dir)) {
        ib_ensure_dir_exists(lib_dir);
    }
    
    // Build static library
    if (build_static) {
        ib_log_message(IB_LOG_INFO, "Building static library: lib%s.a", library_name);
        
        // Make a clean start for the static library
        ib_reset_targets();
        g_config.num_exclude_files = 0;
        
        // Automatically exclude test files (assumed to start with "test_")
        ib_exclude_file("test_*.c");
        ib_exclude_file("build.c");
        
        // Find main source file - either library_name.c or the first file in src/
        char main_source[IB_MAX_PATH] = {0};
        char specific_source[IB_MAX_PATH] = {0};
        sprintf(specific_source, "%s/%s.c", src_dir, library_name);
        
        if (ib_file_exists(specific_source)) {
            strcpy(main_source, specific_source);
        } else {
            // Find the first .c file in src/ directory
            DIR* d = opendir(src_dir);
            if (d) {
                struct dirent* entry;
                while ((entry = readdir(d)) != NULL) {
                    const char* ext = strrchr(entry->d_name, '.');
                    if (ext && strcmp(ext, ".c") == 0) {
                        sprintf(main_source, "%s/%s", src_dir, entry->d_name);
                        break;
                    }
                }
                closedir(d);
            }
        }
        
        if (main_source[0] == '\0') {
            ib_error("Could not find library source files in %s/", src_dir);
            return false;
        }
        
        // Build the static library
        if (!ib_build_static_library(library_name, main_source, NULL)) {
            ib_error("Failed to build static library");
            return false;
        }
        
        // Rename to standard library format if not already
        char lib_path[IB_MAX_PATH] = {0};
        sprintf(lib_path, "%s/lib%s.a", lib_dir, library_name);
        
        // Check if the file exists in this format, if not rename it
        if (!ib_file_exists(lib_path)) {
            char old_path[IB_MAX_PATH] = {0};
            sprintf(old_path, "%s/%s.a", lib_dir, library_name);
            if (ib_file_exists(old_path)) {
                rename(old_path, lib_path);
            }
        }
        
        ib_log_message(IB_LOG_INFO, "Static library created at %s/lib%s.a", lib_dir, library_name);
    }
    
    // Build dynamic library
    if (build_dynamic) {
        ib_log_message(IB_LOG_INFO, "Building dynamic library: lib%s.so", library_name);
        
        // Make a clean start for the dynamic library
        ib_reset_targets();
        g_config.num_exclude_files = 0;
        
        // Automatically exclude test files (assumed to start with "test_")
        ib_exclude_file("test_*.c");
        ib_exclude_file("build.c");
        
        // Find main source file - either library_name.c or the first file in src/
        char main_source[IB_MAX_PATH] = {0};
        char specific_source[IB_MAX_PATH] = {0};
        sprintf(specific_source, "%s/%s.c", src_dir, library_name);
        
        if (ib_file_exists(specific_source)) {
            strcpy(main_source, specific_source);
        } else {
            // Find the first .c file in src/ directory
            DIR* d = opendir(src_dir);
            if (d) {
                struct dirent* entry;
                while ((entry = readdir(d)) != NULL) {
                    const char* ext = strrchr(entry->d_name, '.');
                    if (ext && strcmp(ext, ".c") == 0) {
                        sprintf(main_source, "%s/%s", src_dir, entry->d_name);
                        break;
                    }
                }
                closedir(d);
            }
        }
        
        if (main_source[0] == '\0') {
            ib_error("Could not find library source files in %s/", src_dir);
            return false;
        }
        
        // Build the dynamic library
        if (!ib_build_dynamic_library(library_name, main_source, NULL)) {
            ib_error("Failed to build dynamic library");
            return false;
        }
        
        // Rename to standard library format if not already
        char lib_path[IB_MAX_PATH] = {0};
        sprintf(lib_path, "%s/lib%s.so", lib_dir, library_name);
        
        // Check if the file exists in this format, if not rename it
        if (!ib_file_exists(lib_path)) {
            char old_path[IB_MAX_PATH] = {0};
            sprintf(old_path, "%s/%s.so", lib_dir, library_name);
            if (ib_file_exists(old_path)) {
                rename(old_path, lib_path);
            }
        }
        
        ib_log_message(IB_LOG_INFO, "Dynamic library created at %s/lib%s.so", lib_dir, library_name);
    }
    
    // Build test programs
    if (build_test) {
        // Look for test files
        bool found_test = false;
        ib_log_message(IB_LOG_INFO, "Looking for test files in current directory:");
        DIR* d = opendir(".");
        if (d) {
            struct dirent* entry;
            while ((entry = readdir(d)) != NULL) {
                ib_log_message(IB_LOG_INFO, "  Examining file: %s", entry->d_name);
                if (strncmp(entry->d_name, "test_", 5) == 0) {
                    const char* ext = strrchr(entry->d_name, '.');
                    if (ext && strcmp(ext, ".c") == 0) {
                        found_test = true;
                        ib_log_message(IB_LOG_INFO, "  Found test file: %s", entry->d_name);
                        
                        // Get the base name without extension
                        char test_name[IB_MAX_PATH] = {0};
                        strncpy(test_name, entry->d_name, ext - entry->d_name);
                        
                        ib_log_message(IB_LOG_INFO, "Building test program: %s", test_name);
                        
                        // Check if we have the libraries built
                        char static_lib_path[IB_MAX_PATH] = {0};
                        sprintf(static_lib_path, "lib/lib%s.a", library_name);
                        
                        if (!ib_file_exists(static_lib_path) && !build_static) {
                            ib_log_message(IB_LOG_INFO, "Building static library for test");
                            
                            // Quick build of the static library if it doesn't exist
                            if (!ib_build_static_library(library_name, NULL, NULL)) {
                                ib_error("Failed to build static library for test");
                                continue;
                            }
                        }
                        
                        // Create a fresh configuration for test program
                        ib_reset_config();
                        ib_init();
                        
                        // Set up include path and library
                        ib_add_include_dir("include");
                        ib_add_library_path("lib");
                        ib_add_library(library_name);
                        
                        // Build the test program
                        ib_add_target(test_name, entry->d_name);
                        ib_build();
                        
                        // Create helper script
                        ib_create_run_script(test_name, "lib");
                        
                        ib_log_message(IB_LOG_INFO, "Test program built: %s", test_name);
                        ib_log_message(IB_LOG_INFO, "Run with: ./run_%s.sh", test_name);
                    }
                }
            }
            closedir(d);
        }
        
        if (!found_test) {
            ib_warning("No test files found (expected files starting with 'test_')");
        }
    }
    
    // Provide a summary if we built multiple things
    if ((build_static || build_dynamic) && build_test) {
        // Find which test files we built
        char test_files[IB_MAX_FILES][IB_MAX_PATH] = {0};
        int num_test_files = 0;
        
        DIR* d = opendir(".");
        if (d) {
            struct dirent* entry;
            while ((entry = readdir(d)) != NULL) {
                if (strncmp(entry->d_name, "test_", 5) == 0) {
                    const char* ext = strrchr(entry->d_name, '.');
                    if (ext && strcmp(ext, ".c") == 0) {
                        char test_name[IB_MAX_PATH] = {0};
                        strncpy(test_name, entry->d_name, ext - entry->d_name);
                        
                        // Check if the executable exists
                        if (ib_file_exists(test_name)) {
                            strcpy(test_files[num_test_files++], test_name);
                        }
                    }
                }
            }
            closedir(d);
        }
        
        printf("\n=== Build Summary ===\n");
        
        if (build_static) {
            printf("- Static library:  %s/lib%s.a\n", lib_dir, library_name);
        }
        
        if (build_dynamic) {
            printf("- Dynamic library: %s/lib%s.so\n", lib_dir, library_name);
        }
        
        if (num_test_files > 0) {
            printf("- Test program(s):\n");
            for (int i = 0; i < num_test_files; i++) {
                printf("  * %s (run with: ./run_%s.sh)\n", test_files[i], test_files[i]);
            }
        }
        
        printf("\nBuild completed successfully!\n");
    }
    
    return true;
}

/**
 * Initialize IncludeBuild with default configuration
 */
void ib_init(void) {
    if (g_initialized) {
        ib_error("IncludeBuild already initialized");
        return;
    }
    
    // Set default configuration
    memset(&g_config, 0, sizeof(g_config));
    strcpy(g_config.source_dir, ".");
    strcpy(g_config.build_dir, ".");        // Put executables in current directory
    strcpy(g_config.obj_dir, "buildobjects"); // Put object files in buildobjects directory
    
#ifdef _WIN32
    strcpy(g_config.compiler, "cl");
    strcpy(g_config.compiler_flags, "/nologo /W3 /O2");
#else
    strcpy(g_config.compiler, "gcc");
    strcpy(g_config.compiler_flags, "-Wall -Wextra -O2");
#endif
    
    g_config.verbose = false;
    g_config.color_output = true;
    g_config.num_exclude_files = 0;
    g_config.log_level = IB_LOG_INFO;
    
    // Add current directory to include dirs
    strcpy(g_config.include_dirs[0], ".");
    g_config.num_include_dirs = 1;
    
    g_initialized = true;
    
    ib_log_message(IB_LOG_INFO, "IncludeBuild v%d.%d.%d initialized", 
        IB_VERSION_MAJOR, IB_VERSION_MINOR, IB_VERSION_PATCH);
}

/**
 * Initialize IncludeBuild with custom configuration
 */
void ib_init_with_config(const ib_config* config) {
    if (g_initialized) {
        ib_error("IncludeBuild already initialized");
        return;
    }
    
    // Copy config
    memcpy(&g_config, config, sizeof(ib_config));
    
    g_initialized = true;
    
    ib_log_message(IB_LOG_INFO, "IncludeBuild v%d.%d.%d initialized with custom config", 
        IB_VERSION_MAJOR, IB_VERSION_MINOR, IB_VERSION_PATCH);
}

/**
 * Add an include directory with validation
 */
void ib_add_include_dir(const char* dir) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return;
    }
    
    if (!dir || strlen(dir) == 0) {
        ib_error("Invalid directory path (null or empty)");
        return;
    }
    
    if (g_config.num_include_dirs >= IB_MAX_INCLUDE_DIRS) {
        ib_error("Too many include directories (max: %d)", IB_MAX_INCLUDE_DIRS);
        return;
    }
    
    // Check if directory exists
    if (!ib_file_exists(dir)) {
        ib_warning("Include directory does not exist: %s", dir);
    }
    
    strcpy(g_config.include_dirs[g_config.num_include_dirs++], dir);
    ib_log_message(IB_LOG_DEBUG, "Added include directory: %s", dir);
}

/**
 * Add a build target
 */
void ib_add_target(const char* name, const char* main_source) {
    if (g_num_targets >= IB_MAX_TARGETS) {
        ib_error("Too many targets (max: %d)", IB_MAX_TARGETS);
        return;
    }
    
    ib_target* target = &g_targets[g_num_targets++];
    
    // Use source filename as target name if provided
    if (main_source && strlen(main_source) > 0) {
        // Extract filename without path
        const char* filename = strrchr(main_source, PATH_SEPARATOR);
        if (filename) {
            filename++; // Skip the separator
        } else {
            filename = main_source;
        }
        
        // Copy and remove extension
        strcpy(target->name, filename);
        char* dot = strrchr(target->name, '.');
        if (dot) {
            *dot = '\0';
        }
    } else {
        strcpy(target->name, name);
    }
    
    // Set output path
    ib_join_path(target->output_path, g_config.build_dir, target->name);
    
    #ifdef _WIN32
    strcat(target->output_path, ".exe");
    #endif
    
    strcpy(target->main_source, main_source);
    target->num_files = 0;
    target->is_library = false;
}

/**
 * Build the project
 */
bool ib_build(void) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return false;
    }
    
    ib_log_message(IB_LOG_INFO, "Building project...");
    
    // Create build directory if it doesn't exist
    ib_ensure_dir_exists(g_config.build_dir);
    
    // Create object files directory if it doesn't exist
    ib_ensure_dir_exists(g_config.obj_dir);
    
    // Find all source files
    ib_find_source_files(g_config.source_dir);
    
    // If no targets defined, create a default one
    if (g_num_targets == 0) {
        ib_add_default_target();
    }
    
    // Parse dependencies for all files
    for (int i = 0; i < g_num_files; i++) {
        ib_parse_dependencies(&g_files[i]);
    }
    
    // Compile all files that need rebuilding
    int num_compiled = 0;
    for (int i = 0; i < g_num_files; i++) {
        if (ib_needs_rebuild(&g_files[i])) {
            ib_compile_file(&g_files[i]);
            num_compiled++;
        }
    }
    
    // Link all targets
    for (int i = 0; i < g_num_targets; i++) {
        ib_link_target(&g_targets[i]);
    }
    
    ib_log_message(IB_LOG_INFO, "Build complete. Compiled %d files.", num_compiled);
    
    // Run the executable if requested
    if (g_run_after_build) {
        const char* executable = g_executable_name;
        
        // If no specific executable was specified, use the first target
        if (executable[0] == '\0' && g_num_targets > 0) {
            executable = g_targets[0].name;
        }
        
        if (executable && executable[0] != '\0') {
            ib_log_message(IB_LOG_INFO, "Running executable after build: %s", executable);
            ib_run_executable(executable);
        }
    }
    
    return true;
}

/**
 * Clean the build directory
 */
void ib_clean(void) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return;
    }
    
    ib_log_message(IB_LOG_INFO, "Cleaning object files directory: %s", g_config.obj_dir);
    
    // Clean the object files directory by removing all object files
    DIR* d = opendir(g_config.obj_dir);
    if (!d) {
        ib_error("Failed to open object files directory: %s", g_config.obj_dir);
        return;
    }
    
    struct dirent* entry;
    int num_removed = 0;
    
    while ((entry = readdir(d)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Check file extension
        const char* ext = strrchr(entry->d_name, '.');
        bool is_obj = false;
        if (ext) {
            #ifdef _WIN32
            is_obj = (strcmp(ext, ".obj") == 0);
            #else
            is_obj = (strcmp(ext, ".o") == 0);
            #endif
        }
        
        // If it's an object file, remove it
        if (is_obj) {
            char path[IB_MAX_PATH];
            ib_join_path(path, g_config.obj_dir, entry->d_name);
            
            if (g_config.verbose) {
                ib_log_message(IB_LOG_INFO, "  Removing %s", path);
            }
            
            if (remove(path) == 0) {
                num_removed++;
            } else {
                ib_error("Failed to remove file: %s (%s)", path, strerror(errno));
            }
        }
    }
    
    closedir(d);
    
    ib_log_message(IB_LOG_INFO, "Clean complete. Removed %d files.", num_removed);
}

/**
 * Set the logging level
 */
void ib_set_log_level(ib_log_level level) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return;
    }
    
    g_config.log_level = level;
}

/**
 * Log a message with the specified level
 */
static void ib_log_message(ib_log_level level, const char* fmt, ...) {
    // Only print if our level is sufficient
    if (level > g_config.log_level && level != IB_LOG_ERROR) {
        return;
    }
    
    const char* prefix = "";
    const char* color = "";
    
    switch (level) {
        case IB_LOG_ERROR:
            prefix = "[ERROR] ";
            color = IB_COLOR_RED;
            break;
        case IB_LOG_WARN:
            prefix = "[WARN] ";
            color = IB_COLOR_YELLOW;
            break;
        case IB_LOG_INFO:
            prefix = "[INFO] ";
            color = IB_COLOR_GREEN;
            break;
        case IB_LOG_DEBUG:
            prefix = "[DEBUG] ";
            color = IB_COLOR_CYAN;
            break;
    }
    
    // Print colored prefix
    if (g_config.color_output) {
        fprintf(stdout, "%s%s%s", color, prefix, IB_COLOR_RESET);
    } else {
        fprintf(stdout, "%s", prefix);
    }
    
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    
    fprintf(stdout, "\n");
}

/**
 * Log an error message
 */
static void ib_error(const char* fmt, ...) {
    // Print colored prefix
    if (g_config.color_output) {
        fprintf(stderr, "%s[ERROR] %s", IB_COLOR_RED, IB_COLOR_RESET);
    } else {
        fprintf(stderr, "[ERROR] ");
    }
    
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    
    fprintf(stderr, "\n");
}

/**
 * Log a warning message
 */
static void ib_warning(const char* fmt, ...) {
    if (g_config.log_level < IB_LOG_WARN) {
        return;
    }
    
    // Print colored prefix
    if (g_config.color_output) {
        fprintf(stdout, "%s[WARN] %s", IB_COLOR_YELLOW, IB_COLOR_RESET);
    } else {
        fprintf(stdout, "[WARN] ");
    }
    
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    
    fprintf(stdout, "\n");
}

/**
 * Check if a file exists
 */
static bool ib_file_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/**
 * Get the last modified time of a file
 */
static time_t ib_get_file_mtime(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_mtime;
    }
    return 0;
}

/**
 * Ensure a directory exists, creating it if necessary
 */
static void ib_ensure_dir_exists(const char* path) {
    if (ib_file_exists(path)) {
        return;
    }
    
    ib_log_message(IB_LOG_INFO, "Creating directory: %s", path);
    
    // Create the directory
    #ifdef _WIN32
    if (mkdir(path) != 0) {
    #else
    if (mkdir(path, 0755) != 0) {
    #endif
        ib_error("Failed to create directory: %s (%s)", path, strerror(errno));
        // Exit since we can't continue without the build directory
        exit(1);
    }
}

/**
 * Join two paths together
 */
static char* ib_join_path(char* dest, const char* path1, const char* path2) {
    strcpy(dest, path1);
    
    // Add separator if needed
    size_t len = strlen(dest);
    if (len > 0 && dest[len - 1] != PATH_SEPARATOR && path2[0] != PATH_SEPARATOR) {
        dest[len] = PATH_SEPARATOR;
        dest[len + 1] = '\0';
    } else if (len > 0 && dest[len - 1] == PATH_SEPARATOR && path2[0] == PATH_SEPARATOR) {
        dest[len - 1] = '\0';
    }
    
    strcat(dest, path2);
    return dest;
}

/**
 * Find all source files in a directory
 */
static void ib_find_source_files(const char* dir) {
    DIR* d = opendir(dir);
    if (!d) {
        ib_error("Failed to open directory: %s", dir);
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(d)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char path[IB_MAX_PATH];
        ib_join_path(path, dir, entry->d_name);
        
        struct stat st;
        if (stat(path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // Recursively scan subdirectory
                ib_find_source_files(path);
            } else if (S_ISREG(st.st_mode)) {
                // Check if it's a C/C++ source file
                const char* ext = strrchr(entry->d_name, '.');
                if (ext && (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 || 
                           strcmp(ext, ".cc") == 0 || strcmp(ext, ".cxx") == 0)) {
                    
                    // Auto-exclude common build script files
                    if (strcmp(entry->d_name, "build.c") == 0 ||
                        strcmp(entry->d_name, "build.cpp") == 0 ||
                        strcmp(entry->d_name, "buildsystem.c") == 0 ||
                        strcmp(entry->d_name, "buildsystem.cpp") == 0 ||
                        strcmp(entry->d_name, "make.c") == 0 ||
                        strcmp(entry->d_name, "make.cpp") == 0) {
                        if (g_config.verbose) {
                            ib_log_message(IB_LOG_INFO, "Auto-excluding build script: %s", path);
                        }
                        continue;
                    }
                    
                    // Check if this file is in the exclude list
                    bool excluded = false;
                    for (int i = 0; i < g_config.num_exclude_files; i++) {
                        if (strstr(path, g_config.exclude_files[i]) != NULL) {
                            if (g_config.verbose) {
                                ib_log_message(IB_LOG_INFO, "Excluding file: %s", path);
                            }
                            excluded = true;
                            break;
                        }
                    }
                    
                    if (excluded) {
                        continue;
                    }
                    
                    if (g_num_files >= IB_MAX_FILES) {
                        ib_error("Too many source files (max: %d)", IB_MAX_FILES);
                        break;
                    }
                    
                    // Add file to the list
                    ib_file* file = &g_files[g_num_files++];
                    strcpy(file->path, path);
                    
                    // Set object file path
                    char rel_path[IB_MAX_PATH];
                    if (strncmp(path, g_config.source_dir, strlen(g_config.source_dir)) == 0) {
                        strcpy(rel_path, path + strlen(g_config.source_dir));
                        if (rel_path[0] == PATH_SEPARATOR) {
                            memmove(rel_path, rel_path + 1, strlen(rel_path));
                        }
                    } else {
                        strcpy(rel_path, entry->d_name);
                    }
                    
                    char obj_name[IB_MAX_PATH];
                    strcpy(obj_name, rel_path);
                    char* dot = strrchr(obj_name, '.');
                    if (dot) {
                        #ifdef _WIN32
                        strcpy(dot, ".obj");
                        #else
                        strcpy(dot, ".o");
                        #endif
                    }
                    
                    // Replace path separators with underscores in object name
                    for (char* p = obj_name; *p; p++) {
                        if (*p == PATH_SEPARATOR) {
                            *p = '_';
                        }
                    }
                    
                    ib_join_path(file->obj_path, g_config.obj_dir, obj_name);
                    
                    file->last_modified = st.st_mtime;
                    file->num_deps = 0;
                    file->needs_rebuild = true;
                    
                    if (g_config.verbose) {
                        ib_log_message(IB_LOG_INFO, "Found source file: %s", file->path);
                    }
                }
            }
        }
    }
    
    closedir(d);
}

/**
 * Parse dependencies for a file
 */
static void ib_parse_dependencies(ib_file* file) {
    // Open the file
    FILE* fp = fopen(file->path, "r");
    if (!fp) {
        ib_error("Failed to open file: %s", file->path);
        return;
    }
    
    // Buffer for reading lines
    char line[IB_MAX_PATH];
    
    // Check each line for includes
    while (fgets(line, sizeof(line), fp)) {
        // Look for #include "..."
        char* include_start = strstr(line, "#include \"");
        if (include_start) {
            include_start += 10; // Skip "#include \""
            char* include_end = strchr(include_start, '"');
            if (include_end) {
                *include_end = '\0';
                
                // Search for the included file in include dirs
                for (int i = 0; i < g_config.num_include_dirs; i++) {
                    char include_path[IB_MAX_PATH];
                    ib_join_path(include_path, g_config.include_dirs[i], include_start);
                    
                    // Check if this file exists
                    if (ib_file_exists(include_path)) {
                        // Look for this file in our file list
                        bool found = false;
                        for (int j = 0; j < g_num_files; j++) {
                            if (strcmp(g_files[j].path, include_path) == 0) {
                                // Add as dependency if not already added
                                bool already_dep = false;
                                for (int k = 0; k < file->num_deps; k++) {
                                    if (file->deps[k] == &g_files[j]) {
                                        already_dep = true;
                                        break;
                                    }
                                }
                                
                                if (!already_dep && file->num_deps < IB_MAX_DEPS) {
                                    file->deps[file->num_deps++] = &g_files[j];
                                    if (g_config.verbose) {
                                        ib_log_message(IB_LOG_INFO, "  Dependency: %s -> %s", file->path, include_path);
                                    }
                                }
                                
                                found = true;
                                break;
                            }
                        }
                        
                        if (found) {
                            break;
                        }
                    }
                }
            }
        }
    }
    
    fclose(fp);
}

/**
 * Check if a file needs to be rebuilt
 */
static bool ib_needs_rebuild(ib_file* file) {
    // If object file doesn't exist, rebuild
    if (!ib_file_exists(file->obj_path)) {
        return true;
    }
    
    // Get object file's modification time
    time_t obj_mtime = ib_get_file_mtime(file->obj_path);
    
    // If source file is newer than object file, rebuild
    if (file->last_modified > obj_mtime) {
        return true;
    }
    
    // Check if any dependencies are newer
    for (int i = 0; i < file->num_deps; i++) {
        if (ib_needs_rebuild(file->deps[i]) || file->deps[i]->last_modified > obj_mtime) {
            return true;
        }
    }
    
    return false;
}

/**
 * Compile a source file
 */
static void ib_compile_file(ib_file* file) {
    ib_log_message(IB_LOG_INFO, "Compiling %s", file->path);
    
    char cmd[IB_MAX_CMD];
    int include_flags_len = 0;
    char include_flags[IB_MAX_CMD] = "";
    
    // Build include flags
    for (int i = 0; i < g_config.num_include_dirs; i++) {
        int dir_len = strlen(g_config.include_dirs[i]);
        
        // Check if we would exceed the buffer
        if (include_flags_len + dir_len + 3 >= IB_MAX_CMD) {
            ib_warning("Include flags too long, some directories will be omitted");
            break;
        }
        
#ifdef _WIN32
        include_flags_len += snprintf(include_flags + include_flags_len, IB_MAX_CMD - include_flags_len,
                                     "/I\"%s\" ", g_config.include_dirs[i]);
#else
        include_flags_len += snprintf(include_flags + include_flags_len, IB_MAX_CMD - include_flags_len,
                                     "-I\"%s\" ", g_config.include_dirs[i]);
#endif
    }
    
    // Ensure build directory exists
    char obj_dir[IB_MAX_PATH] = {0};
    strcpy(obj_dir, file->obj_path);
    
    // Get directory part
    char* last_sep = strrchr(obj_dir, PATH_SEPARATOR);
    if (last_sep) {
        *last_sep = '\0';
        ib_ensure_dir_exists(obj_dir);
    }
    
    // Build command
    sprintf(cmd, "%s %s -c %s -o %s", 
        g_config.compiler, g_config.compiler_flags, file->path, file->obj_path);
    
    if (g_config.verbose) {
        ib_log_message(IB_LOG_INFO, "  Command: %s", cmd);
    }
    
    // Execute the command
    FILE* proc = popen(cmd, "r");
    if (!proc) {
        ib_error("Failed to execute command: %s", cmd);
        return;
    }
    
    // Read output
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), proc)) {
        if (g_config.verbose) {
            printf("%s", buffer);
        }
    }
    
    // Check result
    int result = pclose(proc);
    if (result != 0) {
        ib_error("Compilation failed with code %d", result);
    } else {
        file->needs_rebuild = false;
    }
}

/**
 * Link a build target
 */
static void ib_link_target(ib_target* target) {
    ib_log_message(IB_LOG_INFO, "Linking %s", target->name);
    
    // Find all files that belong to this target
    char obj_files[IB_MAX_CMD] = "";
    
    // Create a list of object files (avoiding duplicates)
    bool obj_included[IB_MAX_FILES] = {false};
    
    // Start with the main source file if specified
    if (target->main_source[0]) {
        // Find the corresponding object file
        for (int i = 0; i < g_num_files; i++) {
            if (strcmp(g_files[i].path, target->main_source) == 0) {
                strcat(obj_files, " ");
                strcat(obj_files, g_files[i].obj_path);
                obj_included[i] = true;
                break;
            }
        }
    }
    
    // Add all other object files (avoiding duplicates)
    for (int i = 0; i < g_num_files; i++) {
        if (!obj_included[i] && g_files[i].path[0]) {
            strcat(obj_files, " ");
            strcat(obj_files, g_files[i].obj_path);
        }
    }
    
    // Build command
    char cmd[IB_MAX_CMD];
    
    #ifdef _WIN32
    // MSVC command
    sprintf(cmd, "%s %s /Fe%s %s %s", 
        g_config.compiler, g_config.compiler_flags, target->output_path, obj_files, g_config.linker_flags);
    #else
    // GCC/Clang command
    sprintf(cmd, "%s %s -o %s %s %s", 
        g_config.compiler, g_config.compiler_flags, target->output_path, obj_files, g_config.linker_flags);
    #endif
    
    if (g_config.verbose) {
        ib_log_message(IB_LOG_INFO, "  Command: %s", cmd);
    }
    
    // Execute the command
    FILE* proc = popen(cmd, "r");
    if (!proc) {
        ib_error("Failed to execute command: %s", cmd);
        return;
    }
    
    // Read output
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), proc)) {
        if (g_config.verbose) {
            printf("%s", buffer);
        }
    }
    
    // Check result
    int result = pclose(proc);
    if (result != 0) {
        ib_error("Linking failed with code %d", result);
    } else {
        ib_log_message(IB_LOG_INFO, "Created %s", target->output_path);
    }
}

/**
 * Add a default target based on source files
 */
static void ib_add_default_target(void) {
    if (g_num_files == 0) {
        ib_error("No source files found");
        return;
    }
    
    // Look for a main.c or similar
    const char* main_candidates[] = {
        "main.c", "main.cpp", "main.cc", "Main.c", "Main.cpp",
        "app.c", "app.cpp", "Application.c", "Application.cpp",
        NULL
    };
    
    const char* main_file = NULL;
    
    // First try to find any of the candidates in the root directory
    for (const char** candidate = main_candidates; *candidate; candidate++) {
        char path[IB_MAX_PATH];
        ib_join_path(path, g_config.source_dir, *candidate);
        
        if (ib_file_exists(path)) {
            main_file = path;
            break;
        }
    }
    
    // If not found, look through all files
    if (!main_file) {
        for (int i = 0; i < g_num_files; i++) {
            const char* filename = strrchr(g_files[i].path, PATH_SEPARATOR);
            if (filename) {
                filename++; // Skip the separator
            } else {
                filename = g_files[i].path;
            }
            
            for (const char** candidate = main_candidates; *candidate; candidate++) {
                if (strcmp(filename, *candidate) == 0) {
                    main_file = g_files[i].path;
                    break;
                }
            }
            
            if (main_file) {
                break;
            }
        }
    }
    
    // Create a default target
    const char* target_name;
    char dir_name[IB_MAX_PATH];
    
    // Use directory name as target name
    strcpy(dir_name, g_config.source_dir);
    char* last_sep = strrchr(dir_name, PATH_SEPARATOR);
    if (last_sep) {
        target_name = last_sep + 1;
    } else {
        target_name = dir_name;
    }
    
    // If it's an empty string or just ".", use "app"
    if (!*target_name || strcmp(target_name, ".") == 0) {
        target_name = "app";
    }
    
    ib_add_target(target_name, main_file ? main_file : "");
    ib_log_message(IB_LOG_INFO, "Created default target: %s", target_name);
}

/**
 * Enable or disable verbose output
 */
void ib_set_verbose(bool verbose) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return;
    }
    
    g_config.verbose = verbose;
}

/**
 * Reset the entire configuration and internal state
 */
void ib_reset_config(void) {
    // Reset all internal state
    memset(&g_config, 0, sizeof(g_config));
    ib_reset_targets();
    ib_reset_files();
    g_initialized = false;
}

/**
 * Reset the target list
 */
static void ib_reset_targets(void) {
    g_num_targets = 0;
    memset(g_targets, 0, sizeof(g_targets));
}

/**
 * Reset the files list
 */
static void ib_reset_files(void) {
    g_num_files = 0;
    memset(g_files, 0, sizeof(g_files));
}

/**
 * Execute a shell command safely
 */
static bool ib_execute_command(const char* cmd) {
    ib_log_message(IB_LOG_INFO, "Executing: %s", cmd);
    
    int result = system(cmd);
    if (result != 0) {
        ib_error("Command failed with code %d: %s", result, cmd);
        return false;
    }
    
    return true;
}

/**
 * Add a library to link with
 */
void ib_add_library(const char* library) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return;
    }
    
    if (g_config.num_libraries >= IB_MAX_LIBRARIES) {
        ib_error("Too many libraries (max: %d)", IB_MAX_LIBRARIES);
        return;
    }
    
    strcpy(g_config.libraries[g_config.num_libraries++], library);
    ib_log_message(IB_LOG_DEBUG, "Added library: %s", library);
    
    // Update linker flags
    char new_flags[IB_MAX_CMD] = "";
    strcpy(new_flags, g_config.linker_flags);
    
    #ifdef _WIN32
    strcat(new_flags, " ");
    strcat(new_flags, library);
    strcat(new_flags, ".lib");
    #else
    strcat(new_flags, " -l");
    strcat(new_flags, library);
    #endif
    
    strcpy(g_config.linker_flags, new_flags);
}

/**
 * Add multiple libraries at once
 * @param first The first library name
 * @param ... Additional libraries (NULL-terminated list)
 */
void ib_add_libraries(const char* first, ...) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return;
    }
    
    if (first == NULL || first[0] == '\0') {
        ib_warning("Empty library name specified");
        return;
    }
    
    // Add the first library
    ib_add_library(first);
    
    // Add the rest of the libraries
    va_list args;
    va_start(args, first);
    
    const char* lib;
    while ((lib = va_arg(args, const char*)) != NULL) {
        ib_add_library(lib);
    }
    
    va_end(args);
}

/**
 * Add a library path to search for libraries
 */
void ib_add_library_path(const char* path) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return;
    }
    
    if (g_config.num_library_paths >= IB_MAX_LIBRARY_PATHS) {
        ib_error("Too many library paths (max: %d)", IB_MAX_LIBRARY_PATHS);
        return;
    }
    
    strcpy(g_config.library_paths[g_config.num_library_paths++], path);
    ib_log_message(IB_LOG_DEBUG, "Added library path: %s", path);
    
    // Update linker flags
    char new_flags[IB_MAX_CMD] = "";
    strcpy(new_flags, g_config.linker_flags);
    
    #ifdef _WIN32
    strcat(new_flags, " /LIBPATH:");
    strcat(new_flags, path);
    #else
    strcat(new_flags, " -L");
    strcat(new_flags, path);
    #endif
    
    strcpy(g_config.linker_flags, new_flags);
}

/**
 * Exclude specific files from build
 */
void ib_exclude_file(const char* file) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return;
    }
    
    if (g_config.num_exclude_files >= IB_MAX_FILES) {
        ib_error("Too many excluded files (max: %d)", IB_MAX_FILES);
        return;
    }
    
    strcpy(g_config.exclude_files[g_config.num_exclude_files++], file);
    ib_log_message(IB_LOG_DEBUG, "Excluded file: %s", file);
}

/**
 * Build a static library from source files
 */
bool ib_build_static_library(const char* name, const char* main_source, const char* exclude_file) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return false;
    }
    
    // Create lib directory if it doesn't exist
    ib_ensure_dir_exists("lib");
    
    // Reset internal state
    ib_reset_targets();
    g_config.num_exclude_files = 0;
    
    // Exclude build.c and any other specified files
    ib_exclude_file("build.c");
    if (exclude_file && strlen(exclude_file) > 0) {
        ib_exclude_file(exclude_file);
    }
    
    // Create a target for the library
    ib_target* target = &g_targets[g_num_targets++];
    strcpy(target->name, name);
    if (main_source && strlen(main_source) > 0) {
        strcpy(target->main_source, main_source);
    }
    target->is_library = true;
    
    // Use appropriate compiler flags
    char old_flags[IB_MAX_CMD];
    strcpy(old_flags, g_config.compiler_flags);
    strcpy(g_config.compiler_flags, "-Wall -Wextra -O2 -c -fPIC");
    
    // Compile all source files
    ib_reset_files();
    ib_find_source_files(g_config.source_dir);
    
    // Compile all files that need rebuilding
    for (int i = 0; i < g_num_files; i++) {
        if (ib_needs_rebuild(&g_files[i])) {
            ib_compile_file(&g_files[i]);
        }
    }
    
    // Build the static library using ar with proper lib prefix
    char cmd[IB_MAX_CMD];
    sprintf(cmd, "ar rcs lib/lib%s.a %s/*.o", name, g_config.obj_dir);
    ib_log_message(IB_LOG_INFO, "Executing: %s", cmd);
    
    bool success = ib_execute_command(cmd);
    
    // Restore compiler flags
    strcpy(g_config.compiler_flags, old_flags);
    
    return success;
}

/**
 * Build a dynamic library from source files
 */
bool ib_build_dynamic_library(const char* name, const char* main_source, const char* exclude_file) {
    if (!g_initialized) {
        ib_error("IncludeBuild not initialized. Call ib_init() first.");
        return false;
    }
    
    // Create lib directory if it doesn't exist
    ib_ensure_dir_exists("lib");
    
    // Reset internal state
    ib_reset_targets();
    g_config.num_exclude_files = 0;
    
    // Exclude build.c and any other specified files
    ib_exclude_file("build.c");
    if (exclude_file && strlen(exclude_file) > 0) {
        ib_exclude_file(exclude_file);
    }
    
    // Create a target for the library
    ib_target* target = &g_targets[g_num_targets++];
    strcpy(target->name, name);
    if (main_source && strlen(main_source) > 0) {
        strcpy(target->main_source, main_source);
    }
    target->is_library = true;
    
    // Use appropriate compiler flags
    char old_flags[IB_MAX_CMD];
    strcpy(old_flags, g_config.compiler_flags);
    strcpy(g_config.compiler_flags, "-Wall -Wextra -O2 -c -fPIC");
    
    // Compile all source files
    ib_reset_files();
    ib_find_source_files(g_config.source_dir);
    
    // Compile all files that need rebuilding
    for (int i = 0; i < g_num_files; i++) {
        if (ib_needs_rebuild(&g_files[i])) {
            ib_compile_file(&g_files[i]);
        }
    }
    
    // Build the shared library with proper lib prefix
    char cmd[IB_MAX_CMD];
    sprintf(cmd, "gcc -shared -o lib/lib%s.so %s/*.o", name, g_config.obj_dir);
    ib_log_message(IB_LOG_INFO, "Executing: %s", cmd);
    
    bool success = ib_execute_command(cmd);
    
    // Restore compiler flags
    strcpy(g_config.compiler_flags, old_flags);
    
    return success;
}

/**
 * Create a helper script to run a test program with the library path set
 */
static void ib_create_run_script(const char* program_name, const char* lib_dir) {
    char script_name[IB_MAX_PATH] = {0};
    sprintf(script_name, "run_%s.sh", program_name);
    
    FILE* f = fopen(script_name, "w");
    if (f) {
        fprintf(f, "#!/bin/sh\n");
        fprintf(f, "# Auto-generated script by IncludeBuild\n");
        fprintf(f, "# Sets up library path and runs %s\n\n", program_name);
        fprintf(f, "export LD_LIBRARY_PATH=\"$(pwd)/%s:$LD_LIBRARY_PATH\"\n", lib_dir);
        fprintf(f, "./%s \"$@\"\n", program_name);
        fclose(f);
        
        // Make the script executable
        chmod(script_name, 0755);
    } else {
        ib_warning("Could not create helper script %s", script_name);
    }
}

/**
 * Returns the IncludeBuild version as a string in the format "MAJOR.MINOR.PATCH"
 * @return String containing the version
 */
const char* ib_version(void) {
    static char version[32] = {0};
    if (version[0] == '\0') {
        snprintf(version, sizeof(version), "%d.%d.%d", 
                IB_VERSION_MAJOR, IB_VERSION_MINOR, IB_VERSION_PATCH);
    }
    return version;
}

#endif /* INCLUDEBUILD_H */ 
