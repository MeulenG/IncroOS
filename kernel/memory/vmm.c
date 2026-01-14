#include "vmm.h"
#include "pmm.h"

// Page table structure (simplified for 64-bit)
typedef struct {
    uint64_t entries[512];
} page_table_t;

// Get CR3 register (page directory base)
static inline uint64_t get_cr3(void) {
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

// Set CR3 register
static inline void set_cr3(uint64_t cr3) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(cr3));
}

// Invalidate TLB entry
static inline void invlpg(uint64_t addr) {
    __asm__ volatile("invlpg (%0)" :: "r"(addr) : "memory");
}

// Get page table entry address
// NOTE: This function assumes identity mapping for page tables themselves.
// Currently works because page tables are allocated in the identity-mapped region (0-2MB).
// For higher memory addresses, a recursive mapping or other mechanism would be needed.
static uint64_t* get_page_table_entry(uint64_t virt_addr, bool allocate) {
    uint64_t pml4_index = (virt_addr >> 39) & 0x1FF;
    uint64_t pdp_index = (virt_addr >> 30) & 0x1FF;
    uint64_t pd_index = (virt_addr >> 21) & 0x1FF;
    uint64_t pt_index = (virt_addr >> 12) & 0x1FF;
    
    // Get PML4 table
    page_table_t* pml4 = (page_table_t*)(get_cr3() & ~0xFFF);
    
    // Check/create PDP table
    if (!(pml4->entries[pml4_index] & PTE_PRESENT)) {
        if (!allocate) return 0;
        uint64_t pdp_phys = pmm_alloc_page();
        if (!pdp_phys) return 0;
        
        // Clear the new table
        page_table_t* pdp = (page_table_t*)pdp_phys;
        for (int i = 0; i < 512; i++) {
            pdp->entries[i] = 0;
        }
        
        pml4->entries[pml4_index] = pdp_phys | PTE_PRESENT | PTE_WRITABLE;
    }
    
    page_table_t* pdp = (page_table_t*)(pml4->entries[pml4_index] & ~0xFFF);
    
    // Check/create PD table
    if (!(pdp->entries[pdp_index] & PTE_PRESENT)) {
        if (!allocate) return 0;
        uint64_t pd_phys = pmm_alloc_page();
        if (!pd_phys) return 0;
        
        // Clear the new table
        page_table_t* pd = (page_table_t*)pd_phys;
        for (int i = 0; i < 512; i++) {
            pd->entries[i] = 0;
        }
        
        pdp->entries[pdp_index] = pd_phys | PTE_PRESENT | PTE_WRITABLE;
    }
    
    page_table_t* pd = (page_table_t*)(pdp->entries[pdp_index] & ~0xFFF);
    
    // Check/create PT table
    if (!(pd->entries[pd_index] & PTE_PRESENT)) {
        if (!allocate) return 0;
        uint64_t pt_phys = pmm_alloc_page();
        if (!pt_phys) return 0;
        
        // Clear the new table
        page_table_t* pt = (page_table_t*)pt_phys;
        for (int i = 0; i < 512; i++) {
            pt->entries[i] = 0;
        }
        
        pd->entries[pd_index] = pt_phys | PTE_PRESENT | PTE_WRITABLE;
    }
    
    page_table_t* pt = (page_table_t*)(pd->entries[pd_index] & ~0xFFF);
    
    return &pt->entries[pt_index];
}

void vmm_init(void) {
    // VMM is initialized by the bootloader
    // This function can be used for additional setup if needed
}

bool vmm_map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags) {
    // Validate alignment
    if ((virt_addr & (PAGE_SIZE - 1)) || (phys_addr & (PAGE_SIZE - 1))) {
        return false;
    }
    
    uint64_t* pte = get_page_table_entry(virt_addr, true);
    if (!pte) {
        return false;
    }
    
    *pte = (phys_addr & ~0xFFF) | flags | PTE_PRESENT;
    invlpg(virt_addr);
    
    return true;
}

void vmm_unmap_page(uint64_t virt_addr) {
    uint64_t* pte = get_page_table_entry(virt_addr, false);
    if (pte && (*pte & PTE_PRESENT)) {
        *pte = 0;
        invlpg(virt_addr);
    }
}

uint64_t vmm_get_physical_address(uint64_t virt_addr) {
    uint64_t* pte = get_page_table_entry(virt_addr, false);
    if (!pte || !(*pte & PTE_PRESENT)) {
        return 0;
    }
    
    return (*pte & ~0xFFF) | (virt_addr & 0xFFF);
}

uint64_t vmm_alloc_page(uint64_t virt_addr, uint64_t flags) {
    // Validate alignment
    if (virt_addr & (PAGE_SIZE - 1)) {
        return 0;
    }
    
    uint64_t phys_addr = pmm_alloc_page();
    if (!phys_addr) {
        return 0;
    }
    
    if (!vmm_map_page(virt_addr, phys_addr, flags)) {
        pmm_free_page(phys_addr);
        return 0;
    }
    
    return virt_addr;
}

void vmm_free_page(uint64_t virt_addr) {
    uint64_t phys_addr = vmm_get_physical_address(virt_addr);
    if (phys_addr) {
        vmm_unmap_page(virt_addr);
        pmm_free_page(phys_addr);
    }
}
