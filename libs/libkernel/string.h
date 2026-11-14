#ifndef LIBKERNEL_STRING_H
#define LIBKERNEL_STRING_H

#include <stddef.h>
#include "print.h"

extern size_t strlen(const char* str);
int strcmp(const char* str1, const char* str2);
void* memcpy(void* dest, const void* src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
void* memset(void* dest, int value, size_t n);


#endif
