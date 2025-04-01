#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/build.h"

// Test helper macros
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

// Test logging levels
void test_log_levels() {
    TEST_CASE("log levels");
    
    // Reset config
    ib_init();
    
    // Test the default log level
    TEST_ASSERT(g_config.log_level == IB_LOG_INFO, "Default log level should be INFO");
    
    // Test setting log level
    ib_set_log_level(IB_LOG_DEBUG);
    TEST_ASSERT(g_config.log_level == IB_LOG_DEBUG, "Log level should be DEBUG");
    
    ib_set_log_level(IB_LOG_ERROR);
    TEST_ASSERT(g_config.log_level == IB_LOG_ERROR, "Log level should be ERROR");
    
    printf("  PASS\n");
}

// Test warning handling
void test_warning_handling() {
    TEST_CASE("warning handling");
    
    // Reset config
    ib_init();
    
    // Set to show warnings
    ib_set_log_level(IB_LOG_WARN);
    
    // This should trigger a warning but not an error
    printf("  Adding nonexistent directory (should show warning):\n");
    ib_add_include_dir("this_directory_does_not_exist");
    
    printf("  PASS\n");
}

// Test input validation
void test_input_validation() {
    TEST_CASE("input validation");
    
    // Reset config
    ib_init();
    
    // Test with NULL directory
    printf("  Adding NULL directory (should show error):\n");
    ib_add_include_dir(NULL);
    
    // Test with empty directory
    printf("  Adding empty directory (should show error):\n");
    ib_add_include_dir("");
    
    printf("  PASS\n");
}

// Main function that runs all tests
int main() {
    printf("=== IncludeBuild Logging Tests ===\n");
    
    test_log_levels();
    test_warning_handling();
    test_input_validation();
    
    printf("\nAll tests passed!\n");
    return 0;
} 