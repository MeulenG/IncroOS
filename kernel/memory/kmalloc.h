#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include <stddef.h>
#include <stdint.h>

// Heap allocator for kernel
//
// This module provides dynamic memory allocation for the kernel using
// a first-fit allocation strategy with block coalescing.
// 
// The heap starts at 2MB (after the kernel) and can grow up to 32MB.
// Memory is allocated in 4KB pages and managed using a linked list of blocks.

// Initialize the heap allocator
// Must be called before any kmalloc/kfree operations.
void heap_init(void);

// Allocate memory from the heap
// size: Number of bytes to allocate
// Returns: Pointer to allocated memory, or NULL if allocation fails
// Note: Memory is 16-byte aligned
void* kmalloc(size_t size);

// Free memory back to the heap
// ptr: Pointer to memory previously allocated with kmalloc
// Note: Passing NULL is safe (no-op)
void kfree(void* ptr);

// Allocate and zero memory
// size: Number of bytes to allocate
// Returns: Pointer to zeroed memory, or NULL if allocation fails
void* kzalloc(size_t size);

// Reallocate memory
// ptr: Pointer to previously allocated memory (or NULL for new allocation)
// size: New size in bytes
// Returns: Pointer to reallocated memory, or NULL if allocation fails
// Note: Data is preserved up to the minimum of old and new sizes
void* krealloc(void* ptr, size_t size);

#endif // __KMALLOC_H__
