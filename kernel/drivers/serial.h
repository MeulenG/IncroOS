#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stddef.h>
#include <stdint.h>

// Assembly functions (implemented in serial.asm)
void serial_init(void);
void serial_putchar(char c);

// C wrapper functions
void serial_write(const char* data, size_t size);
void serial_writestring(const char* data);

#endif // __SERIAL_H__
