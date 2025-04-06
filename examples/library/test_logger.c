/**
 * IncludeBuild Library Example - Test Program
 * 
 * This file demonstrates how to use the logger library
 * created with IncludeBuild. It showcases all the features
 * of the logging library, including different log levels,
 * output redirection, and message formatting.
 */

#include <stdio.h>
#include <string.h>
#include "include/logger.h"

int main() {
    printf("========== Logger Library Test ==========\n\n");
    
    // SECTION 1: Basic logging with default settings
    printf("Testing basic logging (INFO level):\n");
    logger_init(LOG_INFO);
    logger_debug("This debug message should NOT appear");
    logger_info("This is an info message");
    logger_warning("This is a warning message");
    logger_error("This is an error message");
    printf("\n");
    
    // SECTION 2: Debug level logging
    printf("Testing DEBUG level logging:\n");
    logger_init(LOG_DEBUG);
    logger_debug("This debug message SHOULD appear now");
    logger_info("Info with formatting: %d, %s", 123, "test");
    printf("\n");
    
    // SECTION 3: File output
    printf("Testing log output to file:\n");
    logger_set_output_file("log_output.txt");
    logger_info("This message goes to the file");
    logger_warning("This warning also goes to the file");
    logger_error("Error with code: %d", 404);
    logger_set_output_file(NULL); // Back to stdout
    logger_info("Back to console output");
    printf("Check log_output.txt for the file output\n\n");
    
    // SECTION 4: Error level only
    printf("Testing ERROR level only:\n");
    logger_init(LOG_ERROR);
    logger_debug("This debug message should NOT appear");
    logger_info("This info message should NOT appear");
    logger_warning("This warning should NOT appear");
    logger_error("This error SHOULD appear");
    logger_fatal("This fatal error SHOULD appear");
    printf("\n");
    
    // Clean up
    logger_cleanup();
    printf("========== Test Complete ==========\n");
    return 0;
}
