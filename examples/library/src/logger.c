/**
 * Logger - Implementation
 * 
 * This file implements the logger library functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include "../include/logger.h"

// Global logger state
static struct {
    LogLevel min_level;
    FILE* output;
    bool is_file_output;
} g_logger = {
    .min_level = LOG_INFO,
    .output = NULL,
    .is_file_output = false
};

// Level name strings
static const char* level_names[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "FATAL"
};

/**
 * Initialize the logger
 */
void logger_init(LogLevel min_level) {
    g_logger.min_level = min_level;
    
    // Default to stdout if not already set
    if (g_logger.output == NULL) {
        g_logger.output = stdout;
        g_logger.is_file_output = false;
    }
}

/**
 * Set output to a file
 */
bool logger_set_output_file(const char* filename) {
    // Close existing file if open
    if (g_logger.is_file_output && g_logger.output != NULL) {
        fclose(g_logger.output);
        g_logger.output = NULL;
        g_logger.is_file_output = false;
    }
    
    // If NULL, use stdout
    if (filename == NULL) {
        g_logger.output = stdout;
        g_logger.is_file_output = false;
        return true;
    }
    
    // Open the file
    g_logger.output = fopen(filename, "a");
    if (g_logger.output == NULL) {
        g_logger.output = stdout; // Fallback to stdout
        return false;
    }
    
    g_logger.is_file_output = true;
    return true;
}

/**
 * Clean up logger resources
 */
void logger_cleanup(void) {
    if (g_logger.is_file_output && g_logger.output != NULL) {
        fclose(g_logger.output);
        g_logger.output = NULL;
    }
}

/**
 * Internal log function
 */
static void log_message(LogLevel level, const char* format, va_list args) {
    // Check if we should log this message
    if (level < g_logger.min_level) {
        return;
    }
    
    // Ensure we have an output
    if (g_logger.output == NULL) {
        g_logger.output = stdout;
        g_logger.is_file_output = false;
    }
    
    // Get current time
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Print the message with timestamp and level
    fprintf(g_logger.output, "[%s][%s] ", timestamp, level_names[level]);
    vfprintf(g_logger.output, format, args);
    fprintf(g_logger.output, "\n");
    
    // Flush to ensure message is written immediately
    fflush(g_logger.output);
}

/**
 * Log a debug message
 */
void logger_debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_DEBUG, format, args);
    va_end(args);
}

/**
 * Log an informational message
 */
void logger_info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_INFO, format, args);
    va_end(args);
}

/**
 * Log a warning message
 */
void logger_warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_WARNING, format, args);
    va_end(args);
}

/**
 * Log an error message
 */
void logger_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_ERROR, format, args);
    va_end(args);
}

/**
 * Log a fatal error message
 */
void logger_fatal(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_FATAL, format, args);
    va_end(args);
} 