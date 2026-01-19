#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include <stddef.h>
#include <stdint.h>


void kmalloc_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);

uint64_t kmalloc_get_used(void);
uint64_t kmalloc_get_free(void);

#endif // __KMALLOC_H__
