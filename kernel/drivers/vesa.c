#include "vesa.h"
#include "basic_font.h"
/*
int vesa_init() {
      

    return 0;
}
*/

int vesa_put_pixel(uint32_t x, uint32_t y, uint32_t color, uint32_t framebuffer, uint16_t width, uint16_t height, uint8_t bpp) {

    if (x >= width || y >= height) {
        // Out of bounds
        return -1;
    }

    uint32_t pixel_offset = (y * width + x) * (bpp / 8);
    uint8_t* pixel_address = (uint8_t*)(framebuffer + pixel_offset);

    // Assuming 32 bits per pixel (4 bytes)
    *(uint32_t*)pixel_address = color;
    return 0;
}

int draw_char(int x, int y, char c, uint32_t color, uint32_t framebuffer, uint16_t width, uint16_t height, uint8_t bpp) {
    for (int row = 0; row < 8; row++) {
        if ((uint8_t)c >= 128) {
            // Out of bounds, bitmap only supports characters in the range 0-127
            return -1;
        }
        uint8_t row_data = font8x8_basic[(uint8_t)c][row];
        for (int col = 0; col < 8; col++) {
            if (row_data & (1 << col)) {
                vesa_put_pixel(x + col, y + row, color, framebuffer, width, height, bpp);
            }
        }
    }
    return 0;
}

int draw_string(int x, int y, const char* str, uint32_t color, uint32_t framebuffer, uint16_t width, uint16_t height, uint8_t bpp) {
    while (*str) {
        draw_char(x, y, *str++, color, framebuffer, width, height, bpp);
        x += 8;
    }
    return 0;
}