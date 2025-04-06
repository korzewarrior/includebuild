/**
 * Logger - A simple logging library
 * 
 * This is a minimalist logging library that supports different
 * log levels, file output, and formatted messages.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

/**
 * Log levels for controlling message verbosity
 */
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3,
    LOG_FATAL = 4
} LogLevel;

/**
 * Initialize the logger with a specific minimum log level
 * Messages below this level will not be logged
 * @param min_level The minimum log level to display
 */
void logger_init(LogLevel min_level);

/**
 * Set the output file for logs
 * By default, logs go to stdout
 * @param filename The file to write logs to, or NULL for stdout
 * @return true if successful
 */
bool logger_set_output_file(const char* filename);

/**
 * Log a message with a specific log level
 * @param level The severity level of the message
 * @param format Format string (printf style)
 * @param ... Variable arguments for format
 */
void logger_log(LogLevel level, const char* format, ...);

/**
 * Convenience functions for different log levels
 */
void logger_debug(const char* format, ...);
void logger_info(const char* format, ...);
void logger_warning(const char* format, ...);
void logger_error(const char* format, ...);
void logger_fatal(const char* format, ...);

/**
 * Clean up logger resources
 */
void logger_cleanup(void);

#endif /* LOGGER_H */ 