#ifndef __VMM_H__
#define __VMM_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Page table entry flags
#define PTE_PRESENT     (1ULL << 0)
#define PTE_WRITABLE    (1ULL << 1)
#define PTE_USER        (1ULL << 2)
#define PTE_WRITETHROUGH (1ULL << 3)
#define PTE_NOCACHE     (1ULL << 4)
#define PTE_ACCESSED    (1ULL << 5)
#define PTE_DIRTY       (1ULL << 6)
#define PTE_HUGEPAGE    (1ULL << 7)
#define PTE_GLOBAL      (1ULL << 8)

// Address masking
#define PAGE_MASK       (~0xFFFULL)
#define PAGE_SIZE       4096

// Initialize the virtual memory manager
void vmm_init(void);

// Map a virtual address to a physical address
// Returns true on success, false on failure
bool vmm_map_page(uint64_t virt, uint64_t phys, uint64_t flags);

// Unmap a virtual address
void vmm_unmap_page(uint64_t virt);

// Get the physical address mapped to a virtual address
// Returns 0 if not mapped
uint64_t vmm_get_physical(uint64_t virt);

// Get current page directory base
uint64_t vmm_get_cr3(void);

#endif // __VMM_H__
