#include "drivers/serial.h"
#include "drivers/keyboard.h"
#include "output/terminal.h"

void kMain(void) {
    serial_init();
    terminal_initialize();
    keyboard_init();

    // Display welcome message with colors
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("IncroOS - 64-bit Kernel\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Keyboard driver initialized\n");
    terminal_writestring("VGA driver with scrolling enabled\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("Type something (keyboard input active):\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    
    // Simple keyboard input loop
    while (1) {
        if (keyboard_has_data()) {
            uint8_t scancode = keyboard_read_scancode();
            char ascii = keyboard_scancode_to_ascii(scancode);
            
            if (ascii != 0) {
                // Echo the character to the terminal
                terminal_putchar(ascii);
            }
        }
    }
}