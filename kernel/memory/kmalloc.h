#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include <stddef.h>
#include <stdint.h>

// Kernel heap allocator
// Provides kmalloc/kfree for dynamic memory allocation

// Initialize the kernel heap
void kmalloc_init(void);

// Allocate memory from kernel heap
// size: number of bytes to allocate
// Returns: pointer to allocated memory, or NULL on failure
void* kmalloc(size_t size);

// Free memory allocated by kmalloc
// ptr: pointer to memory to free
void kfree(void* ptr);

// Get heap statistics
uint64_t kmalloc_get_used(void);
uint64_t kmalloc_get_free(void);

#endif // __KMALLOC_H__
