#ifndef __VMM_H__
#define __VMM_H__

#include <stdint.h>
#include <stdbool.h>

// Page table entry flags
#define PTE_PRESENT    (1 << 0)
#define PTE_WRITABLE   (1 << 1)
#define PTE_USER       (1 << 2)
#define PTE_WRITETHROUGH (1 << 3)
#define PTE_CACHE_DISABLE (1 << 4)

// Virtual Memory Manager
//
// This module manages virtual memory and page tables for the kernel.
// It handles dynamic page table allocation and virtual-to-physical mappings.
//
// NOTE: The current implementation assumes identity mapping for page tables
// themselves (physical address == virtual address for page table structures).
// This works because page tables are allocated in the identity-mapped region (0-2MB).
// Future enhancement: Implement recursive mapping for page tables.

// Initialize the virtual memory manager
// The bootloader has already set up basic paging, this function can be
// used for additional initialization if needed.
void vmm_init(void);

// Map a virtual address to a physical address
// virt_addr: Virtual address to map (must be page-aligned)
// phys_addr: Physical address to map to (must be page-aligned)
// flags: Page table entry flags (PTE_WRITABLE, PTE_USER, etc.)
// Returns: true on success, false on failure
bool vmm_map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);

// Unmap a virtual address
// virt_addr: Virtual address to unmap
void vmm_unmap_page(uint64_t virt_addr);

// Get physical address for a virtual address
// virt_addr: Virtual address to translate
// Returns: Physical address, or 0 if not mapped
uint64_t vmm_get_physical_address(uint64_t virt_addr);

// Allocate and map a new page
// virt_addr: Virtual address to map (must be page-aligned)
// flags: Page table entry flags
// Returns: Virtual address on success, 0 on failure
uint64_t vmm_alloc_page(uint64_t virt_addr, uint64_t flags);

// Free and unmap a page
// virt_addr: Virtual address to free
void vmm_free_page(uint64_t virt_addr);

#endif // __VMM_H__
