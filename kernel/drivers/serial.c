#include "serial.h"

// Use strlen from terminal.h
extern size_t strlen(const char* str);

// Write a string of specific size to serial port
void serial_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        serial_putchar(data[i]);
    }
}

// Write a null-terminated string to serial port
void serial_writestring(const char* data) {
    serial_write(data, strlen(data));
}
