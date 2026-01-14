#ifndef __PMM_H__
#define __PMM_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Physical memory starts after the kernel and boot structures
// We'll use 0x200000 (2MB) as the start of managed physical memory
#define PMM_MEMORY_START    0x200000
#define PMM_MEMORY_SIZE     (32 * 1024 * 1024)  // Manage 32MB initially
#define PMM_PAGE_SIZE       4096
#define PMM_TOTAL_PAGES     (PMM_MEMORY_SIZE / PMM_PAGE_SIZE)

// Initialize the physical memory manager
void pmm_init(void);

// Allocate a physical page frame
// Returns physical address of the page, or 0 if allocation fails
uint64_t pmm_alloc_page(void);

// Free a physical page frame
void pmm_free_page(uint64_t addr);

// Get number of free pages
size_t pmm_get_free_pages(void);

// Get total number of pages
size_t pmm_get_total_pages(void);

#endif // __PMM_H__
