#include "logger.h"
#include "drivers/serial.h"

// External strlen function
extern size_t strlen(const char* str);

// Initialize the logger
void logger_init(void) {
    // Currently nothing to initialize
    // In the future, could set default log level, etc.
}

// Get the log level string
const char* logger_get_level_string(log_level_t level) {
    switch (level) {
        case LOG_TRACE: return "TRACE";
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO";
        case LOG_WARN:  return "WARN";
        case LOG_ERROR: return "ERROR";
        case LOG_PANIC: return "PANIC";
        default:        return "UNKNOWN";
    }
}

// Get CPU ID (placeholder - returns 0 for now, as we're single-core)
uint8_t logger_get_cpu_id(void) {
    // TODO: Implement actual CPU ID detection when SMP support is added
    return 0;
}

// Check if interrupts are enabled by reading RFLAGS
uint8_t logger_are_interrupts_enabled(void) {
    uint64_t flags;
    __asm__ volatile("pushfq; pop %0" : "=r"(flags));
    // Bit 9 is the Interrupt Flag (IF)
    return (flags & (1 << 9)) ? 1 : 0;
}

// Helper to convert integer to string
static void uint_to_string(uint64_t value, char* buffer) {
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    char temp[32];
    int i = 0;
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}

// Helper to extract filename from full path
static const char* extract_filename(const char* path) {
    const char* filename = path;
    const char* p = path;
    
    while (*p) {
        if (*p == '/' || *p == '\\') {
            filename = p + 1;
        }
        p++;
    }
    
    return filename;
}

// Core logging function
void logger_log(log_level_t level, const char* file, int line, const char* message) {
    // Get CPU ID
    uint8_t cpu_id = logger_get_cpu_id();
    
    // Check interrupt status
    uint8_t irq_enabled = logger_are_interrupts_enabled();
    
    // Get level string
    const char* level_str = logger_get_level_string(level);
    
    // Extract filename
    const char* filename = extract_filename(file);
    
    // Build the log message: [CPU#][NOIRQ/IRQ][LEVEL][file:line] message
    
    // Write [CPU#]
    serial_writestring("[CPU");
    char cpu_str[4];
    uint_to_string(cpu_id, cpu_str);
    serial_writestring(cpu_str);
    serial_writestring("]");
    
    // Write [NOIRQ] or [IRQ]
    if (irq_enabled) {
        serial_writestring("[IRQ]");
    } else {
        serial_writestring("[NOIRQ]");
    }
    
    // Write [LEVEL]
    serial_writestring("[");
    serial_writestring(level_str);
    serial_writestring("]");
    
    // Write [file:line]
    serial_writestring("[");
    serial_writestring(filename);
    serial_writestring(":");
    char line_str[16];
    uint_to_string((uint64_t)line, line_str);
    serial_writestring(line_str);
    serial_writestring("]");
    
    // Write message
    serial_writestring(" ");
    serial_writestring(message);
    serial_writestring("\n");
}
