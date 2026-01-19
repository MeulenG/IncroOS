#ifndef __PMM_H__
#define __PMM_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Physical Memory Manager (PMM)
// Manages physical page frames (4KB pages)

#define PAGE_SIZE 4096
#define PAGES_PER_BYTE 8

// Initialize the physical memory manager
// total_memory: total available memory in bytes
void pmm_init(uint64_t total_memory);

// Allocate a physical page frame
// Returns: physical address of allocated page, or 0 on failure
uint64_t pmm_alloc_page(void);

// Free a physical page frame
// addr: physical address of page to free
void pmm_free_page(uint64_t addr);

// Get total number of pages
uint64_t pmm_get_total_pages(void);

// Get number of free pages
uint64_t pmm_get_free_pages(void);

// Get number of used pages
uint64_t pmm_get_used_pages(void);

#endif // __PMM_H__
