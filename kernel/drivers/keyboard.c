#include "keyboard.h"

// Scan Code Set 1 to ASCII mapping
static const char scancode_to_ascii[128] = {
    0,      '\x1B', '1',    '2',    '3',    '4',    '5',    '6',
    '7',    '8',    '9',    '0',    '-',    '=',    '\b',   '\t',
    'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
    'o',    'p',    '[',    ']',    '\n',   0,      'a',    's',
    'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
    '\'',   '`',    0,      '\\',   'z',    'x',    'c',    'v',
    'b',    'n',    'm',    ',',    '.',    '/',    0,      '*',
    0,      ' ',    0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0
};

// IRQ1 parses scan code and this function converts it to ASCII and prints it to the terminal
void handle_keyboard_interrupt(uint8_t scancode) {
    // lookup the ASCII character from the scancode
    char ascii_char = scancode_to_ascii[scancode];
    // print the character to the terminal
    terminal_putchar(ascii_char);
};
