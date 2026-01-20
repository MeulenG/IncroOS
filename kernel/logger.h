#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stddef.h>
#include <stdint.h>

// Log levels
typedef enum {
    LOG_TRACE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARN = 3,
    LOG_ERROR = 4,
    LOG_PANIC = 5
} log_level_t;

// Initialize the logger
void logger_init(void);

// Core logging function
void logger_log(log_level_t level, const char* file, int line, const char* message);

// Convenience macros for logging
#define LOG_TRACE_MSG(msg) logger_log(LOG_TRACE, __FILE__, __LINE__, msg)
#define LOG_DEBUG_MSG(msg) logger_log(LOG_DEBUG, __FILE__, __LINE__, msg)
#define LOG_INFO_MSG(msg) logger_log(LOG_INFO, __FILE__, __LINE__, msg)
#define LOG_WARN_MSG(msg) logger_log(LOG_WARN, __FILE__, __LINE__, msg)
#define LOG_ERROR_MSG(msg) logger_log(LOG_ERROR, __FILE__, __LINE__, msg)
#define LOG_PANIC_MSG(msg) logger_log(LOG_PANIC, __FILE__, __LINE__, msg)

// Helper functions
const char* logger_get_level_string(log_level_t level);
uint8_t logger_get_cpu_id(void);
uint8_t logger_are_interrupts_enabled(void);

#endif // __LOGGER_H__
