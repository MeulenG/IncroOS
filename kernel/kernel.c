#include "kernel.h"
#include "drivers/serial.h"
#include "output/terminal.h"

void kMain(void) {
    serial_init();
    terminal_initialize();

    terminal_writestring("Hello, 64-bit kernel World!\n");
}