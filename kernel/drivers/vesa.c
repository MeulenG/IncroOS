#include "vesa.h"
#include "basic_font.h"

int vesa_put_pixel(uint32_t x, uint32_t y, uint32_t color, struct vbe_mode_info_structure* vesa_info) {

    if (x >= vesa_info->width || y >= vesa_info->height) {
        // Out of bounds
        return -1;
    }

    uint32_t pixel_offset = (y * vesa_info->width + x) * (vesa_info->bpp / 8);
    uint8_t* pixel_address = (uint8_t*)(vesa_info->framebuffer + pixel_offset);

    // Assuming 32 bits per pixel (4 bytes)
    *(uint32_t*)pixel_address = color;
    return 0;
}

int draw_char(int x, int y, char c, uint32_t color, struct vbe_mode_info_structure* vesa_info) {
    for (int row = 0; row < 8; row++) {
        if ((uint8_t)c >= 128) {
            // Out of bounds, bitmap only supports characters in the range 0-127
            return -1;
        }
        uint8_t row_data = font8x8_basic[(uint8_t)c][row];
        for (int col = 0; col < 8; col++) {
            if (row_data & (1 << col)) {
                vesa_put_pixel(x + col, y + row, color, vesa_info);
            }
        }
    }
    return 0;
}

int draw_string(int x, int y, const char* str, uint32_t color, struct vbe_mode_info_structure* vesa_info) {
    while (*str) {
        draw_char(x, y, *str++, color, vesa_info);
        x += 8;
    }
    return 0;
}