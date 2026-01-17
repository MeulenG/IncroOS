#include "pmm.h"
#include "../drivers/serial.h"

// Bitmap to track page allocation status
// Each bit represents one 4KB page frame
static uint8_t* page_bitmap = NULL;
static uint64_t total_pages = 0;
static uint64_t used_pages = 0;

// Helper: Convert page index to bitmap byte and bit
static inline uint64_t page_to_byte(uint64_t page) {
    return page / PAGES_PER_BYTE;
}

static inline uint8_t page_to_bit(uint64_t page) {
    return page % PAGES_PER_BYTE;
}

// Helper: Check if a page is allocated
static bool is_page_allocated(uint64_t page) {
    uint64_t byte = page_to_byte(page);
    uint8_t bit = page_to_bit(page);
    return (page_bitmap[byte] & (1 << bit)) != 0;
}

// Helper: Set page as allocated
static void set_page_allocated(uint64_t page) {
    uint64_t byte = page_to_byte(page);
    uint8_t bit = page_to_bit(page);
    page_bitmap[byte] |= (1 << bit);
}

// Helper: Set page as free
static void set_page_free(uint64_t page) {
    uint64_t byte = page_to_byte(page);
    uint8_t bit = page_to_bit(page);
    page_bitmap[byte] &= ~(1 << bit);
}

void pmm_init(uint64_t total_memory) {
    // Calculate total number of pages
    total_pages = total_memory / PAGE_SIZE;
    
    // Calculate bitmap size in bytes
    uint64_t bitmap_size = (total_pages + PAGES_PER_BYTE - 1) / PAGES_PER_BYTE;
    
    // Place bitmap after kernel code
    // Kernel is at 0x100000 (1MB), we'll place bitmap at 0x200000 (2MB)
    page_bitmap = (uint8_t*)0x200000;
    
    // Initialize bitmap - mark all pages as free
    for (uint64_t i = 0; i < bitmap_size; i++) {
        page_bitmap[i] = 0;
    }
    
    // Mark reserved memory regions as allocated
    // Reserve first 1MB (BIOS, IVT, etc.)
    uint64_t reserved_low = 0x100000 / PAGE_SIZE;  // First 1MB
    for (uint64_t i = 0; i < reserved_low; i++) {
        set_page_allocated(i);
        used_pages++;
    }
    
    // Reserve kernel area (1MB to 2MB)
    uint64_t kernel_pages = 0x100000 / PAGE_SIZE;  // 1MB for kernel
    for (uint64_t i = reserved_low; i < reserved_low + kernel_pages; i++) {
        set_page_allocated(i);
        used_pages++;
    }
    
    // Reserve bitmap area
    uint64_t bitmap_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint64_t bitmap_start_page = 0x200000 / PAGE_SIZE;
    for (uint64_t i = bitmap_start_page; i < bitmap_start_page + bitmap_pages; i++) {
        set_page_allocated(i);
        used_pages++;
    }
    
    serial_writestring("[PMM] Physical Memory Manager initialized\n");
}

uint64_t pmm_alloc_page(void) {
    // Find first free page
    for (uint64_t page = 0; page < total_pages; page++) {
        if (!is_page_allocated(page)) {
            set_page_allocated(page);
            used_pages++;
            return page * PAGE_SIZE;  // Return physical address
        }
    }
    
    // No free pages available
    return 0;
}

void pmm_free_page(uint64_t addr) {
    uint64_t page = addr / PAGE_SIZE;
    
    if (page >= total_pages) {
        return;  // Invalid page
    }
    
    if (is_page_allocated(page)) {
        set_page_free(page);
        used_pages--;
    }
}

uint64_t pmm_get_total_pages(void) {
    return total_pages;
}

uint64_t pmm_get_free_pages(void) {
    return total_pages - used_pages;
}

uint64_t pmm_get_used_pages(void) {
    return used_pages;
}
