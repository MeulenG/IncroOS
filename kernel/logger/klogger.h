#ifndef __KLOGGER_H__
#define __KLOGGER_H__
#include <stddef.h>
#include <stdint.h>

typedef struct KLogger KLogger;

typedef enum {
    LOG_TRACE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARN = 3,
    LOG_ERROR = 4,
    LOG_PANIC = 5
} log_level_t;

KLogger *logger_init(void);

// helper function
// trailing underscores to signal an internal function
uint8_t isInterruptsEnabled__(void);
uint8_t getCpuId__(void);

KLogger LOG_TRACE(KLogger *self, const char *format, ...);
KLogger LOG_DEBUG(KLogger *self, const char *format, ...);
KLogger LOG_INFO(KLogger *self, const char *format, ...);
KLogger LOG_WARN(KLogger *self, const char *format, ...);
KLogger LOG_ERROR(KLogger *self, const char *format, ...);
KLogger LOG_PANIC(KLogger *self, const char *format, ...);

#endif // __KLOGGER_H__
