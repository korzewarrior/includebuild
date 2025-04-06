/**
 * Simple app to demonstrate IncludeBuild's logging features
 * 
 * This is the target application that gets built in the logging example.
 * It doesn't do anything special - it just serves as a target to build.
 */

#include <stdio.h>
#include <time.h>

// A simple function to demonstrate different log levels in an application
void demo_logging() {
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    printf("[DEBUG]   %s - Initializing application components\n", timestamp);
    printf("[INFO]    %s - Application started successfully\n", timestamp);
    printf("[WARNING] %s - Configuration file not found, using defaults\n", timestamp);
    printf("[ERROR]   %s - Failed to connect to database after 3 attempts\n", timestamp);
}

int main() {
    printf("\n========== IncludeBuild Logging Demo ==========\n\n");
    printf("This program was built using IncludeBuild's logging features.\n");
    printf("The build script demonstrated various log levels:\n");
    printf("  - DEBUG: Most verbose, shows all messages\n");
    printf("  - INFO: Normal operational messages\n");
    printf("  - WARNING: Issues that don't prevent operation\n");
    printf("  - ERROR: Critical issues that need attention\n\n");
    
    printf("Example application logs:\n");
    demo_logging();
    
    printf("\n==============================================\n");
    return 0;
} 