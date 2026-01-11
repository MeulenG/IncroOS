#include "serial.h"

// Helper function to get string length
static size_t serial_strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

// Write a string of specific size to serial port
void serial_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        serial_putchar(data[i]);
    }
}

// Write a null-terminated string to serial port
void serial_writestring(const char* data) {
    serial_write(data, serial_strlen(data));
}
