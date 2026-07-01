#include "vesa.h"

// this makes no sense
// I want the init function, maybe vesa_put_pixel should take in extra params for this, keep for now
/*
int vesa_init() {
    struct vbe_mode_info_structure* vesa_info = (struct vbe_mode_info_structure*)0x7000;
    uint32_t framebuffer = vesa_info->framebuffer;
    uint16_t width = vesa_info->width;
    uint16_t height = vesa_info->height;
    uint8_t bpp = vesa_info->bpp;   

    return 0;
}
*/

int vesa_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    struct vbe_mode_info_structure* vesa_info = (struct vbe_mode_info_structure*)0x7000;
    uint32_t framebuffer = vesa_info->framebuffer;
    uint16_t width = vesa_info->width;
    uint16_t height = vesa_info->height;
    uint8_t bpp = vesa_info->bpp;

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