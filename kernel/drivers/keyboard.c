#include "keyboard.h"

// US QWERTY keyboard scancode to ASCII mapping (scancode set 1)
// Index is the scancode, value is the ASCII character
static const char scancode_to_ascii_map[] = {
    0,    27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=', '\b',  '\t',
    'q', 'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n',    0,  'a',  's',
    'd', 'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',  '`',    0, '\\',  'z',  'x',  'c',  'v',
    'b', 'n',  'm',  ',',  '.',  '/',    0,  '*',    0,  ' ',    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  '-',    0,    0,    0,  '+',    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

// Shifted characters mapping
static const char scancode_to_ascii_shifted[] = {
    0,    27,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '+', '\b', '\t',
    'Q', 'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n',    0,  'A',  'S',
    'D', 'F',  'G',  'H',  'J',  'K',  'L',  ':', '\"',  '~',    0,  '|',  'Z',  'X',  'C',  'V',
    'B', 'N',  'M',  '<',  '>',  '?',    0,  '*',    0,  ' ',    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  '-',    0,    0,    0,  '+',    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

// Keyboard state
static bool shift_pressed = false;
static bool ctrl_pressed = false;
static bool alt_pressed = false;

// Initialize the PS/2 keyboard
void keyboard_init(void) {
    // Clear the keyboard buffer by reading any pending data
    while (keyboard_has_data()) {
        inb(KEYBOARD_DATA_PORT);
    }
}

// Check if keyboard has data available
bool keyboard_has_data(void) {
    uint8_t status = inb(KEYBOARD_STATUS_PORT);
    return (status & KEYBOARD_STATUS_OUTPUT_FULL) != 0;
}

// Read a scancode from the keyboard
// Returns 0 if no data available
uint8_t keyboard_read_scancode(void) {
    if (!keyboard_has_data()) {
        return 0;
    }
    return inb(KEYBOARD_DATA_PORT);
}

// Convert a scancode to ASCII character
// Returns 0 for non-printable keys or if scancode is invalid
char keyboard_scancode_to_ascii(uint8_t scancode) {
    // Check for key release (bit 7 set)
    bool key_released = (scancode & 0x80) != 0;
    scancode &= 0x7F;  // Remove the release bit
    
    // Handle modifier keys
    if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT) {
        shift_pressed = !key_released;
        return 0;
    }
    
    if (scancode == KEY_LCTRL) {
        ctrl_pressed = !key_released;
        return 0;
    }
    
    if (scancode == KEY_LALT) {
        alt_pressed = !key_released;
        return 0;
    }
    
    // Only process key presses, not releases
    if (key_released) {
        return 0;
    }
    
    // Convert scancode to ASCII
    if (scancode < sizeof(scancode_to_ascii_map)) {
        if (shift_pressed) {
            return scancode_to_ascii_shifted[scancode];
        } else {
            return scancode_to_ascii_map[scancode];
        }
    }
    
    return 0;  // Unknown scancode
}
