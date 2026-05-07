#include <stdarg.h>
#include <stdint.h>
#include "../drivers/serial.h"
#include "../output/terminal.h"

static void emit_char(char c)         { 
    if (c == '\n')
    {
        serial_putchar('\r');
    }
    // Output to both serial port and VGA
    // Serial Port
    serial_putchar(c);

    // VGA
    terminal_putchar(c);
}

static void emit_str(const char *s)   { /* loop emit_char until null */
    do
    {
        emit_char(*s);
    } while (s != NULL);
}

static void emit_decimal(int64_t val)   { 
    if (val == 0) {
        emit_char('0');
        return;
    }
    
    if (val < 0) {
        emit_char('-');
        val = -val;
    }

    // buffer enough for 64-bit max(20 digits)
    char digits[20];
    int i = 0;
    while (val > 0) {
        digits[i++] = '0' + val % 10;
        val /= 10;
    }

    while (i > 0) {
        emit_char(digits[--i]);
    }
}

static void emit_unsigned(uint64_t val) {
    int buffer[100];
    if (val == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    // buffer enough for 64-bit max(20 digits)
    uint64_t digits[20];
    uint64_t i = 0;
    while (val > 0) {
        digits[i++] = '0' + val % 10;
        val /= 10;
    }

    while (i > 0) {
        emit_char(digits[--i]);
    } 
}

static void emit_hex(uint64_t val, int min_width) { 
    // 64-bit value is at most 16 hex digits
    const char* digits[16];
    int64_t i = 0;
    do
    {
        digits[i++] = "0123456789abcdef"[val & 0xF];
        val >>= 4;
    } while (val > 0);
    
    while (i > 0) {
        emit_char(digits[--i]);
    }
}

void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            emit_char(*fmt++);
            continue;
        }

        fmt++;  // skip the %
        switch (*fmt) {
            case 'd': emit_decimal(va_arg(args, int)); break;
            case 'u': emit_unsigned(va_arg(args, unsigned int)); break;
            case 'x': emit_hex(va_arg(args, unsigned int), 0); break;
            case 's': emit_str(va_arg(args, const char *)); break;
            case 'c': emit_char((char) va_arg(args, int)); break;
            case 'p': {
                emit_str("0x");
                emit_hex((uintptr_t) va_arg(args, void *), 16);
                break;
            }
            case '%': emit_char('%'); break;
            default:
                // unknown specifier, emit raw
                emit_char('%');
                emit_char(*fmt);
                break;
        }
        fmt++;  // past the specifier
    }
    
    va_end(args);
}
