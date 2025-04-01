#include <stdio.h>
#include <string.h>
#include "include/logger.h"

// Example of using the logger library
int main() {
    printf("Logger Library Test\n");
    printf("==================\n\n");
    
    // Initialize the logger with minimum level set to DEBUG
    logger_init(LOG_DEBUG);
    printf("Logger initialized with minimum level: DEBUG\n\n");
    
    // Test different log levels
    printf("Testing different log levels:\n");
    logger_debug("This is a debug message");
    logger_info("This is an info message");
    logger_warning("This is a warning message");
    logger_error("This is an error message");
    logger_fatal("This is a fatal error message");
    printf("\n");
    
    // Test changing the minimum log level
    printf("Changing minimum log level to WARNING...\n");
    logger_init(LOG_WARNING);
    printf("These messages should NOT appear (below WARNING level):\n");
    logger_debug("Debug message - should not be visible");
    logger_info("Info message - should not be visible");
    printf("These messages should appear (WARNING and above):\n");
    logger_warning("Warning message - should be visible");
    logger_error("Error message - should be visible");
    logger_fatal("Fatal message - should be visible");
    printf("\n");
    
    // Test log to file
    printf("Testing logging to file...\n");
    logger_init(LOG_INFO);
    if (logger_set_output_file("log_output.txt")) {
        logger_info("This message should go to the file");
        logger_warning("This warning should go to the file");
        logger_error("This error should go to the file");
        printf("Wrote log messages to log_output.txt\n");
    } else {
        printf("Failed to open log file\n");
    }
    
    // Reset to console output
    logger_set_output_file(NULL);
    logger_info("Back to console output");
    
    // Test complex message formatting
    logger_info("Complex message with formatting: %d, %s, %.2f", 42, "test string", 3.14159);
    
    // Clean up
    logger_cleanup();
    printf("\nLogger test completed successfully!\n");
    
    return 0;
} 