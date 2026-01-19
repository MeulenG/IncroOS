#ifndef __VMM_H__
#define __VMM_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define PT_PRESENT    (1 << 0)
#define PT_WRITABLE   (1 << 1)
#define PT_USER       (1 << 2)
#define PT_WRITETHROUGH (1 << 3)
#define PT_CACHE_DISABLE (1 << 4)

typedef uint64_t pte_t;

void vmm_init(void);

// Returns: true on success, false on failure
bool vmm_map_page(uint64_t virt, uint64_t phys, uint64_t flags);

// virtual address to unmap
void vmm_unmap_page(uint64_t virt);

// returns the physical address, or 0 if not mapped
uint64_t vmm_get_physical(uint64_t virt);

// returns true if the virtual address is mapped, false otherwise
bool vmm_is_mapped(uint64_t virt);

#endif // __VMM_H__
