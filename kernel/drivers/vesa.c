#include "vesa.h"
#include "basic_font.h"

static uint32_t g_vesa_framebuffer = 0;
static uint16_t g_vesa_width = 0;
static uint16_t g_vesa_height = 0;
static uint8_t g_vesa_bpp = 0;

int vesa_init() {
    struct vbe_mode_info_structure* vesa_info = (struct vbe_mode_info_structure*)0x7000;
    g_vesa_framebuffer = vesa_info->framebuffer;
    g_vesa_width = vesa_info->width;
    g_vesa_height = vesa_info->height;
    g_vesa_bpp = vesa_info->bpp;
    
    return 0;
}

int vesa_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= g_vesa_width || y >= g_vesa_height) {
        // Out of bounds
        return -1;
    }

    uint32_t pixel_offset = (y * g_vesa_width + x) * (g_vesa_bpp / 8);
    uint8_t* pixel_address = (uint8_t*)(g_vesa_framebuffer + pixel_offset);

    // Assuming 32 bits per pixel (4 bytes)
    *(uint32_t*)pixel_address = color;
    return 0;
}

int draw_char(int x, int y, char c, uint32_t color) {
    for (int row = 0; row < 8; row++) {
        if ((uint8_t)c >= 128) {
            // Out of bounds, bitmap only supports characters in the range 0-127
            return -1;
        }
        uint8_t row_data = font8x8_basic[(uint8_t)c][row];
        for (int col = 0; col < 8; col++) {
            if (row_data & (1 << col)) {
                vesa_put_pixel(x + col, y + row, color);
            }
        }
    }
    return 0;
}

int draw_string(int x, int y, const char* str, uint32_t color) {
    while (*str) {
        draw_char(x, y, *str++, color);
        x += 8;
    }
    return 0;
}