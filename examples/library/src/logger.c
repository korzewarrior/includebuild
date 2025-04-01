#include "../include/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

// Static variables
static LogLevel current_min_level = LOG_INFO;
static FILE* log_file = NULL;
static int use_stdout = 1;

// Names for log levels
static const char* level_names[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "FATAL"
};

// Color codes for different log levels (ANSI escape sequences)
static const char* level_colors[] = {
    "\x1b[36m", // Cyan for DEBUG
    "\x1b[32m", // Green for INFO
    "\x1b[33m", // Yellow for WARNING
    "\x1b[31m", // Red for ERROR
    "\x1b[35m"  // Magenta for FATAL
};

// Reset color code
static const char* color_reset = "\x1b[0m";

void logger_init(LogLevel min_level) {
    current_min_level = min_level;
    use_stdout = 1;
    log_file = NULL;
}

int logger_set_output_file(const char* filename) {
    // If we already have a file open, close it
    if (log_file != NULL && !use_stdout) {
        fclose(log_file);
        log_file = NULL;
    }
    
    // If filename is NULL, use stdout
    if (filename == NULL) {
        use_stdout = 1;
        return 1;
    }
    
    // Open the log file
    log_file = fopen(filename, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        use_stdout = 1;
        return 0;
    }
    
    use_stdout = 0;
    return 1;
}

void logger_log(LogLevel level, const char* format, ...) {
    // Check if the level is sufficient to log
    if (level < current_min_level) {
        return;
    }
    
    // Get current time
    time_t now = time(NULL);
    struct tm* time_info = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);
    
    // Format the log message
    FILE* output = use_stdout ? stdout : log_file;
    
    // Add colors if using stdout
    if (use_stdout) {
        fprintf(output, "%s[%s][%s]%s ", level_colors[level], time_str, level_names[level], color_reset);
    } else {
        fprintf(output, "[%s][%s] ", time_str, level_names[level]);
    }
    
    // Print the message with variable arguments
    va_list args;
    va_start(args, format);
    vfprintf(output, format, args);
    va_end(args);
    
    // Add a newline if not already present
    if (format[0] == '\0' || format[strlen(format) - 1] != '\n') {
        fprintf(output, "\n");
    }
    
    // Flush to ensure message is written
    fflush(output);
}

// Convenience functions
void logger_debug(const char* format, ...) {
    if (LOG_DEBUG < current_min_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    logger_log(LOG_DEBUG, "%s", buffer);
}

void logger_info(const char* format, ...) {
    if (LOG_INFO < current_min_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    logger_log(LOG_INFO, "%s", buffer);
}

void logger_warning(const char* format, ...) {
    if (LOG_WARNING < current_min_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    logger_log(LOG_WARNING, "%s", buffer);
}

void logger_error(const char* format, ...) {
    if (LOG_ERROR < current_min_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    logger_log(LOG_ERROR, "%s", buffer);
}

void logger_fatal(const char* format, ...) {
    if (LOG_FATAL < current_min_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    logger_log(LOG_FATAL, "%s", buffer);
}

void logger_cleanup(void) {
    if (log_file != NULL && !use_stdout) {
        fclose(log_file);
        log_file = NULL;
    }
} 