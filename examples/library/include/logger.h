#ifndef LOGGER_H
#define LOGGER_H

/**
 * Log levels for controlling message verbosity
 */
typedef enum {
    LOG_DEBUG,   // Detailed debugging information
    LOG_INFO,    // General information messages
    LOG_WARNING, // Warning messages
    LOG_ERROR,   // Error messages
    LOG_FATAL    // Critical errors
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
 * @return 1 on success, 0 on failure
 */
int logger_set_output_file(const char* filename);

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