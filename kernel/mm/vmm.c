#include "vmm.h"
#include "pmm.h"

// Helper to read CR3
static inline uint64_t read_cr3(void) {
    uint64_t value;
    __asm__ volatile("mov %%cr3, %0" : "=r"(value));
    return value;
}

// Helper to write CR3
static inline void write_cr3(uint64_t value) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(value) : "memory");
}

// Helper to invalidate TLB entry
static inline void invlpg(uint64_t addr) {
    __asm__ volatile("invlpg (%0)" :: "r"(addr) : "memory");
}

// Extract page table indices from virtual address
#define PML4_INDEX(addr) (((addr) >> 39) & 0x1FF)
#define PDPT_INDEX(addr) (((addr) >> 30) & 0x1FF)
#define PD_INDEX(addr)   (((addr) >> 21) & 0x1FF)
#define PT_INDEX(addr)   (((addr) >> 12) & 0x1FF)

// Get physical address from page table entry
#define PTE_ADDR(pte) ((pte) & PAGE_MASK)

void vmm_init(void) {
    // Virtual memory is already set up by the bootloader
    // We just need to be able to manage it
}

// Helper function to get or create a page table
static uint64_t* get_or_create_table(uint64_t* parent_table, size_t index) {
    uint64_t entry = parent_table[index];
    
    if (!(entry & PTE_PRESENT)) {
        // Allocate a new page table (already cleared by pmm_alloc_page)
        uint64_t new_table_phys = pmm_alloc_page();
        if (new_table_phys == 0) {
            return 0;  // Allocation failed
        }
        
        // Set the parent entry
        parent_table[index] = new_table_phys | PTE_PRESENT | PTE_WRITABLE;
        entry = parent_table[index];
    }
    
    return (uint64_t*)PTE_ADDR(entry);
}

bool vmm_map_page(uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t cr3 = read_cr3();
    uint64_t* pml4 = (uint64_t*)cr3;
    
    // Get or create page tables at each level
    uint64_t* pdpt = get_or_create_table(pml4, PML4_INDEX(virt));
    if (pdpt == 0) return false;
    
    uint64_t* pd = get_or_create_table(pdpt, PDPT_INDEX(virt));
    if (pd == 0) return false;
    
    uint64_t* pt = get_or_create_table(pd, PD_INDEX(virt));
    if (pt == 0) return false;
    
    // Set the page table entry
    pt[PT_INDEX(virt)] = (phys & PAGE_MASK) | flags;
    
    // Invalidate TLB entry
    invlpg(virt);
    
    return true;
}

void vmm_unmap_page(uint64_t virt) {
    uint64_t cr3 = read_cr3();
    uint64_t* pml4 = (uint64_t*)cr3;
    
    // Walk the page tables
    uint64_t pml4_entry = pml4[PML4_INDEX(virt)];
    if (!(pml4_entry & PTE_PRESENT)) return;
    
    uint64_t* pdpt = (uint64_t*)PTE_ADDR(pml4_entry);
    uint64_t pdpt_entry = pdpt[PDPT_INDEX(virt)];
    if (!(pdpt_entry & PTE_PRESENT)) return;
    
    uint64_t* pd = (uint64_t*)PTE_ADDR(pdpt_entry);
    uint64_t pd_entry = pd[PD_INDEX(virt)];
    if (!(pd_entry & PTE_PRESENT)) return;
    
    uint64_t* pt = (uint64_t*)PTE_ADDR(pd_entry);
    
    // Clear the page table entry
    pt[PT_INDEX(virt)] = 0;
    
    // Invalidate TLB entry
    invlpg(virt);
}

uint64_t vmm_get_physical(uint64_t virt) {
    uint64_t cr3 = read_cr3();
    uint64_t* pml4 = (uint64_t*)cr3;
    
    // Walk the page tables
    uint64_t pml4_entry = pml4[PML4_INDEX(virt)];
    if (!(pml4_entry & PTE_PRESENT)) return 0;
    
    uint64_t* pdpt = (uint64_t*)PTE_ADDR(pml4_entry);
    uint64_t pdpt_entry = pdpt[PDPT_INDEX(virt)];
    if (!(pdpt_entry & PTE_PRESENT)) return 0;
    
    uint64_t* pd = (uint64_t*)PTE_ADDR(pdpt_entry);
    uint64_t pd_entry = pd[PD_INDEX(virt)];
    if (!(pd_entry & PTE_PRESENT)) return 0;
    
    uint64_t* pt = (uint64_t*)PTE_ADDR(pd_entry);
    uint64_t pt_entry = pt[PT_INDEX(virt)];
    if (!(pt_entry & PTE_PRESENT)) return 0;
    
    return PTE_ADDR(pt_entry) | (virt & 0xFFF);
}

uint64_t vmm_get_cr3(void) {
    return read_cr3();
}
