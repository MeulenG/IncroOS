#ifndef __HEAP_H__
#define __HEAP_H__

#include <stddef.h>
#include <stdint.h>

// Initialize the heap allocator
void heap_init(void);

// Allocate memory from the kernel heap
void* kmalloc(size_t size);

// Free memory back to the kernel heap
void kfree(void* ptr);

// Get heap statistics
size_t heap_get_used(void);
size_t heap_get_free(void);

#endif // __HEAP_H__
