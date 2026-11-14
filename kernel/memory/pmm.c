#include "pmm.h"
#include "../drivers/serial.h"
#include "../boot_info.h"
#include "../../libs/libkernel/print.h"

// The bootloader identity maps the first 4GB and the VMM reaches page tables through that
// map, so only memory below 4GB is handed out
#define PMM_MEMORY_LIMIT 0x100000000ULL
#define PMM_BITMAP_ADDRESS 0x140000
#define PMM_KERNEL_START 0x100000
#define PMM_KERNEL_RESERVED 0x80000

// End of the kernel image including .bss, from the linker
extern char _end[];

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

// Marks whole pages in [start, end): freeing rounds inwards, reserving rounds outwards
static void pmm_mark_range(uint64_t start, uint64_t end, bool allocated) {
    uint64_t first = allocated ? start / PAGE_SIZE : (start + PAGE_SIZE - 1) / PAGE_SIZE;
    uint64_t last = allocated ? (end + PAGE_SIZE - 1) / PAGE_SIZE : end / PAGE_SIZE;
    if (last > total_pages) {
        last = total_pages;
    }
    for (uint64_t page = first; page < last; page++) {
        if (allocated && !is_page_allocated(page)) {
            set_page_allocated(page);
            used_pages++;
        } else if (!allocated && is_page_allocated(page)) {
            set_page_free(page);
            used_pages--;
        }
    }
}

// Sizes itself from the E820 memory map stage2 collected
int pmm_init() {
    const struct boot_info* info = g_boot_info;

    // The highest usable address below 4GB sets the size of the bitmap
    uint64_t top = 0;
    for (uint32_t i = 0; i < info->memory_map_count; i++) {
        const struct e820_entry* entry = &info->memory_map[i];
        if (entry->type == E820_USABLE && entry->base + entry->length > top) {
            top = entry->base + entry->length;
        }
    }
    if (top > PMM_MEMORY_LIMIT) {
        top = PMM_MEMORY_LIMIT;
    }
    if (top == 0) {
        serial_writestring("[PMM] The memory map has no usable memory\n");
        return -1;
    }
    if ((uint64_t)_end > PMM_BITMAP_ADDRESS) {
        serial_writestring("[PMM] The kernel image runs into the page bitmap at 0x140000\n");
        return -1;
    }

    total_pages = top / PAGE_SIZE;
    uint64_t bitmap_size = (total_pages + PAGES_PER_BYTE - 1) / PAGES_PER_BYTE;
    page_bitmap = (uint8_t*)PMM_BITMAP_ADDRESS;

    // Everything starts in use, so holes in the map (MMIO, the framebuffer) are never handed out
    for (uint64_t i = 0; i < bitmap_size; i++) {
        page_bitmap[i] = 0xFF;
    }
    used_pages = total_pages;

    for (uint32_t i = 0; i < info->memory_map_count; i++) {
        const struct e820_entry* entry = &info->memory_map[i];
        if (entry->type == E820_USABLE) {
            pmm_mark_range(entry->base, entry->base + entry->length, false);
        }
    }
    // A reserved region that overlaps a usable one wins
    for (uint32_t i = 0; i < info->memory_map_count; i++) {
        const struct e820_entry* entry = &info->memory_map[i];
        if (entry->type != E820_USABLE) {
            pmm_mark_range(entry->base, entry->base + entry->length, true);
        }
    }

    // BIOS data, stage2, the boot page tables and the stack live below 1MB
    pmm_mark_range(0, PMM_KERNEL_START, true);
    pmm_mark_range(PMM_KERNEL_START, PMM_KERNEL_START + PMM_KERNEL_RESERVED, true);
    pmm_mark_range(PMM_BITMAP_ADDRESS, PMM_BITMAP_ADDRESS + bitmap_size, true);

    kprintf("[PMM] Physical Memory Manager initialized: %lu MB below 0x%lx, %lu pages free\n",
            top / (1024 * 1024), top, total_pages - used_pages);
    return 0;
}

static bool pmm_in_usable_memory(uint64_t addr) {
    for (uint32_t i = 0; i < g_boot_info->memory_map_count; i++) {
        const struct e820_entry* entry = &g_boot_info->memory_map[i];
        if (entry->type == E820_USABLE && addr >= entry->base && addr + PAGE_SIZE <= entry->base + entry->length) {
            return true;
        }
    }
    return false;
}

int validate_pmm(void) {
    const struct boot_info* info = g_boot_info;
    if (pmm_get_free_pages() == 0) {
        serial_writestring("PMM validation failed: no free pages\n");
        return -1;
    }

    // Nothing the firmware did not report as usable RAM may be free, nor the framebuffer
    for (uint32_t i = 0; i < info->memory_map_count; i++) {
        const struct e820_entry* entry = &info->memory_map[i];
        if (entry->type == E820_USABLE || entry->length == 0) {
            continue;
        }
        uint64_t first = entry->base / PAGE_SIZE;
        uint64_t last = (entry->base + entry->length - 1) / PAGE_SIZE;
        if ((first < total_pages && !is_page_allocated(first)) || (last < total_pages && !is_page_allocated(last))) {
            serial_writestring("PMM validation failed: a reserved region is marked free\n");
            return -1;
        }
    }
    uint64_t framebuffer_page = info->vbe_mode_info.framebuffer / PAGE_SIZE;
    if (framebuffer_page < total_pages && !is_page_allocated(framebuffer_page)) {
        serial_writestring("PMM validation failed: the framebuffer is marked free\n");
        return -1;
    }

    // A fresh page has to be usable RAM above the kernel and the bitmap
    uint64_t free_before = pmm_get_free_pages();
    uint64_t page = pmm_alloc_page();
    if (page < PMM_KERNEL_START + PMM_KERNEL_RESERVED || !pmm_in_usable_memory(page)) {
        kprintf("PMM validation failed: handed out page 0x%lx, which is not free usable memory\n", page);
        return -1;
    }
    pmm_free_page(page);
    if (pmm_get_free_pages() != free_before) {
        serial_writestring("PMM validation failed: free pages did not return to baseline\n");
        return -1;
    }

    serial_writestring("PMM validation passed!\n");
    return 0;
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

int pmm_free_page(uint64_t addr) {
    uint64_t page = addr / PAGE_SIZE;

    if (page >= total_pages) {
        return -1; // Invalid page number
    }

    if (is_page_allocated(page)) {
        set_page_free(page);
        used_pages--;
    }
    return 0;
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
