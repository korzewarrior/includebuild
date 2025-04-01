#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/build.h"

// Define some test helper macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s\n", message); \
            printf("  at %s:%d\n", __FILE__, __LINE__); \
            exit(1); \
        } \
    } while (0)

#define TEST_CASE(name) \
    printf("\nRunning test: %s\n", name)

// Test configuration initialization
void test_init() {
    TEST_CASE("ib_init");
    
    // Test default initialization
    ib_init();
    
    // Verify default values
    TEST_ASSERT(g_config.verbose == false, "Default verbose should be false");
    TEST_ASSERT(g_config.color_output == true, "Default color_output should be true");
    TEST_ASSERT(strcmp(g_config.compiler, "gcc") == 0 || 
                strcmp(g_config.compiler, "cl") == 0, "Default compiler should be gcc or cl");
    TEST_ASSERT(g_config.num_include_dirs == 1, "Default num_include_dirs should be 1");
    TEST_ASSERT(g_config.num_exclude_files == 0, "Default num_exclude_files should be 0");
    
    printf("  PASS\n");
}

// Test include directory management
void test_include_dirs() {
    TEST_CASE("ib_add_include_dir");
    
    // Reset config
    ib_init();
    
    // The first include dir is set in ib_init
    int initial_include_dirs = g_config.num_include_dirs;
    
    // Test adding include directories
    ib_add_include_dir("include");
    TEST_ASSERT(g_config.num_include_dirs == initial_include_dirs + 1, "Should have added 1 include directory");
    TEST_ASSERT(strcmp(g_config.include_dirs[initial_include_dirs], "include") == 0, "Include dir should be 'include'");
    
    ib_add_include_dir("src/include");
    TEST_ASSERT(g_config.num_include_dirs == initial_include_dirs + 2, "Should have added 2 include directories");
    TEST_ASSERT(strcmp(g_config.include_dirs[initial_include_dirs + 1], "src/include") == 0, "Include dir should be 'src/include'");
    
    // Test max include dirs (this shouldn't crash)
    for (int i = 0; i < IB_MAX_INCLUDE_DIRS; i++) {
        char dir[64];
        sprintf(dir, "dir%d", i);
        ib_add_include_dir(dir);
    }
    
    printf("  PASS\n");
}

// Test target management
void test_targets() {
    TEST_CASE("ib_add_target");
    
    // Reset config
    ib_init();
    
    // Add a target
    ib_add_target("test_app", "main.c");
    TEST_ASSERT(g_num_targets == 1, "Should have 1 target");
    TEST_ASSERT(strcmp(g_targets[0].name, "test_app") == 0, "Target name should be 'test_app'");
    TEST_ASSERT(strcmp(g_targets[0].main_source, "main.c") == 0, "Main source should be 'main.c'");
    
    printf("  PASS\n");
}

// Test building (just the initialization, not actual compiling)
void test_build_setup() {
    TEST_CASE("build setup");
    
    // Reset config
    ib_init();
    
    // Set a dummy source directory that doesn't exist
    strcpy(g_config.source_dir, "nonexistent_dir");
    
    // Add a dummy target
    ib_add_target("dummy", "nonexistent.c");
    
    // Just make sure build setup doesn't crash
    bool result = ib_build();
    
    // This should "fail" because the file doesn't exist, but not crash
    TEST_ASSERT(result == false, "Build should fail with nonexistent file");
    
    printf("  PASS\n");
}

// Main function that runs all tests
int main() {
    printf("=== IncludeBuild Basic Tests ===\n");
    
    test_init();
    test_include_dirs();
    test_targets();
    test_build_setup();
    
    printf("\nAll tests passed!\n");
    return 0;
} 