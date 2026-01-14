#ifndef __PMM_H__
#define __PMM_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Page size is 4KB
#define PAGE_SIZE 4096
#define BITMAP_SIZE 32768  // Support up to 128MB of RAM (32768 * 4KB)

// Physical Memory Manager - Page Frame Allocator
// 
// This module manages physical memory allocation using a bitmap-based approach.
// Each bit in the bitmap represents one 4KB page frame.
// The allocator reserves:
//   - Pages below 1MB (BIOS, VGA, etc.)
//   - Pages from 1MB-2MB (kernel code and data)

// Initialize the physical memory manager
// memory_size: Total physical memory available in bytes
void pmm_init(uint64_t memory_size);

// Allocate a single page frame
// Returns: Physical address of allocated page, or 0 if no memory available
// Note: Returned address is always page-aligned (multiple of 4KB)
uint64_t pmm_alloc_page(void);

// Free a single page frame
// page_addr: Physical address of the page to free (must be page-aligned)
// Note: Will not free pages in reserved regions (below 2MB)
void pmm_free_page(uint64_t page_addr);

// Get total memory size in bytes
uint64_t pmm_get_total_memory(void);

// Get used memory size in bytes
uint64_t pmm_get_used_memory(void);

// Get free memory size in bytes
uint64_t pmm_get_free_memory(void);

#endif // __PMM_H__
