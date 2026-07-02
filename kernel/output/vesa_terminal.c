#include "vesa_terminal.h"
#include "../drivers/serial.h"


void vesa_terminal_initialize(void) {
    vesa_init();
}

void vesa_terminal_putchar(char c, uint32_t color) {
    // For simplicity, let's assume we are drawing characters at a fixed position
    static int x = 0;
    static int y = 0;

    if (c == '\n') {
        x = 0;
        // Move down by character height
        y += 8;
        return;
    }

    draw_char(x, y, c, color); // Draw in the specified color
    x += 8; // Move right by character width
}

void vesa_terminal_write(const char* data, size_t size, uint32_t color) {
    for (size_t i = 0; i < size; i++) {
        vesa_terminal_putchar(data[i], color);
    }
}

void vesa_terminal_writestring(const char* data, uint32_t color) {
    vesa_terminal_write(data, strlen(data), color);
    // Also output to serial port
    serial_writestring(data);
}
