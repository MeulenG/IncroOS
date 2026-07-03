#ifndef VESA_TERMINAL_H
#define VESA_TERMINAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../drivers/vesa.h"
#include "../../libs/libkernel/string.h"

enum vesa_color {
    VESA_COLOR_BLACK = 0x000000,
    VESA_COLOR_BLUE = 0x0000FF,
    VESA_COLOR_GREEN = 0x00FF00,
    VESA_COLOR_CYAN = 0x00FFFF,
    VESA_COLOR_RED = 0xFF0000,
    VESA_COLOR_MAGENTA = 0xFF00FF,
    VESA_COLOR_BROWN = 0xA52A2A,
    VESA_COLOR_LIGHT_GREY = 0xD3D3D3,
    VESA_COLOR_DARK_GREY = 0xA9A9A9,
    VESA_COLOR_LIGHT_BLUE = 0xADD8E6,
    VESA_COLOR_LIGHT_GREEN = 0x90EE90,
    VESA_COLOR_LIGHT_CYAN = 0xE0FFFF,
    VESA_COLOR_LIGHT_RED = 0xFFA07A,
    VESA_COLOR_LIGHT_MAGENTA = 0xEE82EE,
    VESA_COLOR_YELLOW = 0xFFFF00,
    VESA_COLOR_WHITE = 0xFFFFFF
};

extern uint8_t terminal_color;

void vesa_terminal_initialize(void);
void vesa_terminal_putchar(char c, uint32_t color);
void vesa_terminal_write(const char* data, size_t size, uint32_t color);
void vesa_terminal_writestring(const char* data, uint32_t color);


#endif
