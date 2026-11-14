#include "klogger.h"


struct KLogger {
    log_level_t level;
};

uint8_t isInterruptsEnabled__(void) {
    uint64_t flags;
    __asm__ volatile("pushfq; pop %0" : "=r"(flags));
    // Bit 9 is the Interrupt Flag
    return (flags & (1 << 9)) ? 1 : 0;
}

uint8_t getCpuId__(void) {
    uint32_t cpu_id;
    __asm__ volatile("mov %%gs:0, %0" : "=r"(cpu_id));
    return (uint8_t)cpu_id;
}

KLogger *logger_init(void) { 
    KLogger *logger = (KLogger *)malloc(sizeof(KLogger));
    return logger;
}

KLogger LOG_TRACE(KLogger *self, const char *format, ...) {
    isInterruptsEnabled__();
    getCpuId__();
    
}