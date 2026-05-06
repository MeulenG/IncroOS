#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stddef.h>
#include <stdint.h>

// C wrapper functions
int serial_init();
void serial_putchar(char c);

void serial_write(const char* data, size_t size);
void serial_writestring(const char* data);

#endif // __SERIAL_H__
