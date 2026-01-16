#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <stdint.h>
#include <stdbool.h>

// PS/2 Keyboard ports
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_COMMAND_PORT 0x64

// Keyboard status register bits
#define KEYBOARD_STATUS_OUTPUT_FULL  0x01
#define KEYBOARD_STATUS_INPUT_FULL   0x02

// Special keys
#define KEY_ESCAPE      0x01
#define KEY_BACKSPACE   0x0E
#define KEY_TAB         0x0F
#define KEY_ENTER       0x1C
#define KEY_LCTRL       0x1D
#define KEY_LSHIFT      0x2A
#define KEY_RSHIFT      0x36
#define KEY_LALT        0x38
#define KEY_CAPSLOCK    0x3A

// Function prototypes
void keyboard_init(void);
uint8_t keyboard_read_scancode(void);
char keyboard_scancode_to_ascii(uint8_t scancode);
bool keyboard_has_data(void);

// Assembly functions (defined in keyboard.asm)
extern uint8_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t value);

#endif // __KEYBOARD_H__
