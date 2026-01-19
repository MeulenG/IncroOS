#include "vmm.h"
#include "pmm.h"
#include "../drivers/serial.h"

#define PML4_INDEX(addr) (((addr) >> 39) & 0x1FF)
#define PDPT_INDEX(addr) (((addr) >> 30) & 0x1FF)
#define PD_INDEX(addr)   (((addr) >> 21) & 0x1FF)
#define PT_INDEX(addr)   (((addr) >> 12) & 0x1FF)

#define PT_ADDR_MASK 0x000FFFFFFFFFF000ULL

static pte_t* pml4 = NULL;

static inline uint64_t get_cr3(void) {
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

static inline void set_cr3(uint64_t cr3) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(cr3));
}

static pte_t* alloc_page_table(void) {
    uint64_t phys = pmm_alloc_page();
    if (phys == 0) {
        return NULL;
    }

    pte_t* table = (pte_t*)phys;
    for (int i = 0; i < 512; i++) {
        table[i] = 0;
    }

    return table;
}

static pte_t* get_or_create_table(pte_t* parent, uint64_t index, uint64_t flags) {
    if (parent[index] & PT_PRESENT) {
        return (pte_t*)(parent[index] & PT_ADDR_MASK);
    }

    pte_t* table = alloc_page_table();
    if (table == NULL) {
        return NULL;
    }

    parent[index] = ((uint64_t)table) | flags | PT_PRESENT;
    return table;
}

void vmm_init(void) {
    uint64_t cr3 = get_cr3();
    pml4 = (pte_t*)cr3;

    serial_writestring("[VMM] Virtual Memory Manager initialized\n");
}

bool vmm_map_page(uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t pml4_idx = PML4_INDEX(virt);
    uint64_t pdpt_idx = PDPT_INDEX(virt);
    uint64_t pd_idx = PD_INDEX(virt);
    uint64_t pt_idx = PT_INDEX(virt);

    pte_t* pdpt = get_or_create_table(pml4, pml4_idx, PT_WRITABLE);
    if (pdpt == NULL) return false;

    pte_t* pd = get_or_create_table(pdpt, pdpt_idx, PT_WRITABLE);
    if (pd == NULL) {
        return false;
    }
    pte_t* pt = get_or_create_table(pd, pd_idx, PT_WRITABLE);
    if (pt == NULL) {
        return false;
    }
    pt[pt_idx] = (phys & PT_ADDR_MASK) | flags | PT_PRESENT;

    __asm__ volatile("invlpg (%0)" :: "r"(virt) : "memory");

    return true;
}

void vmm_unmap_page(uint64_t virt) {
    uint64_t pml4_idx = PML4_INDEX(virt);
    uint64_t pdpt_idx = PDPT_INDEX(virt);
    uint64_t pd_idx = PD_INDEX(virt);
    uint64_t pt_idx = PT_INDEX(virt);

    if (!(pml4[pml4_idx] & PT_PRESENT)) { 
        return; 
    }
    pte_t* pdpt = (pte_t*)(pml4[pml4_idx] & PT_ADDR_MASK);

    if (!(pdpt[pdpt_idx] & PT_PRESENT)) {
        return;
    }
    pte_t* pd = (pte_t*)(pdpt[pdpt_idx] & PT_ADDR_MASK);

    if (!(pd[pd_idx] & PT_PRESENT)) {
        return;
    }
    pte_t* pt = (pte_t*)(pd[pd_idx] & PT_ADDR_MASK);

    pt[pt_idx] = 0;

    __asm__ volatile("invlpg (%0)" :: "r"(virt) : "memory");
}

uint64_t vmm_get_physical(uint64_t virt) {
    uint64_t pml4_idx = PML4_INDEX(virt);
    uint64_t pdpt_idx = PDPT_INDEX(virt);
    uint64_t pd_idx = PD_INDEX(virt);
    uint64_t pt_idx = PT_INDEX(virt);

    if (!(pml4[pml4_idx] & PT_PRESENT)) {
        return 0;
    }
    pte_t* pdpt = (pte_t*)(pml4[pml4_idx] & PT_ADDR_MASK);

    if (!(pdpt[pdpt_idx] & PT_PRESENT)) {
        return 0;
    }
    pte_t* pd = (pte_t*)(pdpt[pdpt_idx] & PT_ADDR_MASK);

    if (!(pd[pd_idx] & PT_PRESENT)) {
        return 0;
    }
    pte_t* pt = (pte_t*)(pd[pd_idx] & PT_ADDR_MASK);

    if (!(pt[pt_idx] & PT_PRESENT)) {
        return 0;
    }

    uint64_t phys_base = pt[pt_idx] & PT_ADDR_MASK;
    uint64_t offset = virt & 0xFFF;
    return phys_base + offset;
}

bool vmm_is_mapped(uint64_t virt) {
    return vmm_get_physical(virt) != 0;
}
