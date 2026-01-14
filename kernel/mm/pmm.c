#include "pmm.h"

// Bitmap to track free/used pages
// Each bit represents one 4KB page
static uint8_t pmm_bitmap[PMM_TOTAL_PAGES / 8];
static size_t pmm_free_pages = PMM_TOTAL_PAGES;

// For initial allocations, we'll use a simple bump allocator
// from the identity-mapped region to avoid circular dependency
#define INITIAL_ALLOC_START 0x10000  // Start at 64KB (after page tables at 0x1000-0x4FFF)
#define INITIAL_ALLOC_END   0x100000 // End before kernel at 1MB
static uint64_t initial_alloc_ptr = INITIAL_ALLOC_START;

// Helper functions for bitmap manipulation
static inline void set_bit(size_t bit) {
    pmm_bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void clear_bit(size_t bit) {
    pmm_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline bool test_bit(size_t bit) {
    return (pmm_bitmap[bit / 8] & (1 << (bit % 8))) != 0;
}

void pmm_init(void) {
    // Initialize all pages as free
    for (size_t i = 0; i < sizeof(pmm_bitmap); i++) {
        pmm_bitmap[i] = 0;
    }
    pmm_free_pages = PMM_TOTAL_PAGES;
}

uint64_t pmm_alloc_page(void) {
    // For initial allocations (when setting up page tables),
    // use the identity-mapped region
    if (initial_alloc_ptr + PMM_PAGE_SIZE <= INITIAL_ALLOC_END) {
        uint64_t addr = initial_alloc_ptr;
        initial_alloc_ptr += PMM_PAGE_SIZE;
        
        // Clear the page
        uint8_t* page = (uint8_t*)addr;
        for (size_t i = 0; i < PMM_PAGE_SIZE; i++) {
            page[i] = 0;
        }
        
        return addr;
    }
    
    // Find first free page
    for (size_t i = 0; i < PMM_TOTAL_PAGES; i++) {
        if (!test_bit(i)) {
            // Mark as used
            set_bit(i);
            pmm_free_pages--;
            
            // Return physical address
            return PMM_MEMORY_START + (i * PMM_PAGE_SIZE);
        }
    }
    
    // No free pages available
    return 0;
}

void pmm_free_page(uint64_t addr) {
    // Don't free pages from initial allocation region
    if (addr >= INITIAL_ALLOC_START && addr < INITIAL_ALLOC_END) {
        return;
    }
    
    // Validate address
    if (addr < PMM_MEMORY_START || 
        addr >= PMM_MEMORY_START + PMM_MEMORY_SIZE ||
        (addr & (PMM_PAGE_SIZE - 1)) != 0) {
        return;  // Invalid address
    }
    
    // Calculate page index
    size_t page_index = (addr - PMM_MEMORY_START) / PMM_PAGE_SIZE;
    
    // Mark as free if it was used
    if (test_bit(page_index)) {
        clear_bit(page_index);
        pmm_free_pages++;
    }
}

size_t pmm_get_free_pages(void) {
    return pmm_free_pages;
}

size_t pmm_get_total_pages(void) {
    return PMM_TOTAL_PAGES;
}
