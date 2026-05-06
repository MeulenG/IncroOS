#include "serial.h"
#include "../cpu/io.h"

#define COM1_PORT 0x3f8          // COM1

int serial_init() {
   outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
   outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(COM1_PORT + 0, 0x01);    // Set divisor to 1 for 115200 baud
   outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
   outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   outb(COM1_PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
   outb(COM1_PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

   // Check if serial is faulty
   if(inb(COM1_PORT + 0) != 0xAE) {
      return 1;
   }

   // If serial is not faulty set it in normal operation mode
   outb(COM1_PORT + 4, 0x0F);
   return 0;
}

void serial_putchar(char c) {
    while (!(inb(COM1_PORT + 5) & 0x20)) { /* wait for THR empty */ }
    outb(COM1_PORT + 0, c);
}

// Use strlen from terminal.h
extern size_t strlen(const char* str);


// Write a string of specific size to serial port
void serial_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        // Convert LF to CRLF for proper serial terminal output
        if (data[i] == '\n') {
            serial_putchar('\r');
        }
        serial_putchar(data[i]);
    }
}

// Write a null-terminated string to serial port
void serial_writestring(const char* data) {
    serial_write(data, strlen(data));
}
