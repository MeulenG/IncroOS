#include "pmm.h"

// Bitmap to track page allocation (1 = used, 0 = free)
static uint8_t memory_bitmap[BITMAP_SIZE];
static uint64_t total_pages = 0;
static uint64_t used_pages = 0;
static uint64_t kernel_end = 0x200000;  // Kernel ends at 2MB (current identity mapping limit)

// Helper function to set a bit in the bitmap
static void bitmap_set(uint64_t bit) {
    if (bit / 8 >= BITMAP_SIZE) return;  // Bounds check
    memory_bitmap[bit / 8] |= (1 << (bit % 8));
}

// Helper function to clear a bit in the bitmap
static void bitmap_clear(uint64_t bit) {
    if (bit / 8 >= BITMAP_SIZE) return;  // Bounds check
    memory_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

// Helper function to test a bit in the bitmap
static bool bitmap_test(uint64_t bit) {
    if (bit / 8 >= BITMAP_SIZE) return true;  // Out of bounds = treated as used
    return (memory_bitmap[bit / 8] & (1 << (bit % 8))) != 0;
}

void pmm_init(uint64_t memory_size) {
    // Calculate total pages
    total_pages = memory_size / PAGE_SIZE;
    if (total_pages > BITMAP_SIZE * 8) {
        total_pages = BITMAP_SIZE * 8;  // Cap at bitmap size
    }
    
    // Initialize bitmap - all pages free
    for (uint64_t i = 0; i < BITMAP_SIZE; i++) {
        memory_bitmap[i] = 0;
    }
    
    // Mark pages below 1MB as used (BIOS, VGA, etc.)
    for (uint64_t i = 0; i < (0x100000 / PAGE_SIZE); i++) {
        bitmap_set(i);
        used_pages++;
    }
    
    // Mark kernel pages as used (1MB to 2MB)
    for (uint64_t i = (0x100000 / PAGE_SIZE); i < (kernel_end / PAGE_SIZE); i++) {
        bitmap_set(i);
        used_pages++;
    }
}

uint64_t pmm_alloc_page(void) {
    // Find first free page
    for (uint64_t i = (kernel_end / PAGE_SIZE); i < total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            used_pages++;
            return i * PAGE_SIZE;
        }
    }
    
    return 0;  // No free pages
}

void pmm_free_page(uint64_t page_addr) {
    uint64_t page_index = page_addr / PAGE_SIZE;
    
    // Validate page alignment
    if (page_addr & (PAGE_SIZE - 1)) {
        return;  // Not page-aligned
    }
    
    // Don't free kernel or reserved memory
    if (page_index < (kernel_end / PAGE_SIZE)) {
        return;
    }
    
    if (page_index < total_pages && bitmap_test(page_index)) {
        bitmap_clear(page_index);
        used_pages--;
    }
}

uint64_t pmm_get_total_memory(void) {
    return total_pages * PAGE_SIZE;
}

uint64_t pmm_get_used_memory(void) {
    return used_pages * PAGE_SIZE;
}

uint64_t pmm_get_free_memory(void) {
    return (total_pages - used_pages) * PAGE_SIZE;
}
