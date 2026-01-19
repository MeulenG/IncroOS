#ifndef __VMM_H__
#define __VMM_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Virtual Memory Manager (VMM)
// Manages page tables and virtual-to-physical mappings

// Page table entry flags
#define PT_PRESENT    (1 << 0)
#define PT_WRITABLE   (1 << 1)
#define PT_USER       (1 << 2)
#define PT_WRITETHROUGH (1 << 3)
#define PT_CACHE_DISABLE (1 << 4)

// Page table structure (64-bit)
typedef uint64_t pte_t;

// Initialize the virtual memory manager
void vmm_init(void);

// Map a virtual address to a physical address
// virt: virtual address to map
// phys: physical address to map to
// flags: page table entry flags
// Returns: true on success, false on failure
bool vmm_map_page(uint64_t virt, uint64_t phys, uint64_t flags);

// Unmap a virtual address
// virt: virtual address to unmap
void vmm_unmap_page(uint64_t virt);

// Get physical address for a virtual address
// virt: virtual address
// Returns: physical address, or 0 if not mapped
uint64_t vmm_get_physical(uint64_t virt);

// Check if a virtual address is mapped
// virt: virtual address to check
// Returns: true if mapped, false otherwise
bool vmm_is_mapped(uint64_t virt);

#endif // __VMM_H__
