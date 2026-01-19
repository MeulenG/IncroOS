#include "pmm.h"
#include "../drivers/serial.h"

static uint8_t* page_bitmap = NULL;
static uint64_t total_pages = 0;
static uint64_t used_pages = 0;

static inline uint64_t page_to_byte(uint64_t page) {
    return page / PAGES_PER_BYTE;
}

static inline uint8_t page_to_bit(uint64_t page) {
    return page % PAGES_PER_BYTE;
}

static bool is_page_allocated(uint64_t page) {
    uint64_t byte = page_to_byte(page);
    uint8_t bit = page_to_bit(page);
    return (page_bitmap[byte] & (1 << bit)) != 0;
}

static void set_page_allocated(uint64_t page) {
    uint64_t byte = page_to_byte(page);
    uint8_t bit = page_to_bit(page);
    page_bitmap[byte] |= (1 << bit);
}

static void set_page_free(uint64_t page) {
    uint64_t byte = page_to_byte(page);
    uint8_t bit = page_to_bit(page);
    page_bitmap[byte] &= ~(1 << bit);
}

void pmm_init(uint64_t total_memory) {
    total_pages = total_memory / PAGE_SIZE;
    uint64_t bitmap_size = (total_pages + PAGES_PER_BYTE - 1) / PAGES_PER_BYTE;
    page_bitmap = (uint8_t*)0x140000;

    for (uint64_t i = 0; i < bitmap_size; i++) {
        page_bitmap[i] = 0;
    }

    uint64_t reserved_low = 0x100000 / PAGE_SIZE;
    for (uint64_t i = 0; i < reserved_low; i++) {
        set_page_allocated(i);
        used_pages++;
    }

    uint64_t kernel_pages = 0x80000 / PAGE_SIZE;
    for (uint64_t i = reserved_low; i < reserved_low + kernel_pages; i++) {
        set_page_allocated(i);
        used_pages++;
    }

    uint64_t bitmap_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint64_t bitmap_start_page = 0x140000 / PAGE_SIZE;
    for (uint64_t i = bitmap_start_page; i < bitmap_start_page + bitmap_pages; i++) {
        set_page_allocated(i);
        used_pages++;
    }

    serial_writestring("[PMM] Physical Memory Manager initialized\n");
}

uint64_t pmm_alloc_page(void) {
    for (uint64_t page = 0; page < total_pages; page++) {
        if (!is_page_allocated(page)) {
            set_page_allocated(page);
            used_pages++;
            return page * PAGE_SIZE;
        }
    }

    return 0;
}

void pmm_free_page(uint64_t addr) {
    uint64_t page = addr / PAGE_SIZE;

    if (page >= total_pages) {
        return;
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
