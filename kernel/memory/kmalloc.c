#include "kmalloc.h"
#include "pmm.h"
#include "vmm.h"
#include "../drivers/serial.h"
#include <stdbool.h>

// Memory block header
typedef struct block_header {
    size_t size;
    bool is_free;
    struct block_header* next;
} block_header_t;

// The heap lives just above the bootloader's 4GB identity map (PDPT[4] is unused),
// and grows on demand with frames from the pmm
#define HEAP_VIRT_START   0x100000000ULL
#define HEAP_VIRT_MAX     (HEAP_VIRT_START + 0x10000000ULL) // 256MB
#define HEAP_INITIAL_SIZE 0x10000
#define BLOCK_HEADER_SIZE sizeof(block_header_t)

static block_header_t* heap_start = NULL;
static uint64_t heap_end = HEAP_VIRT_START; // first unmapped address
static uint64_t total_allocated = 0;

static size_t align_size(size_t size) {
    return (size + 15) & ~15;
}

// maps at least size more bytes at the end of the heap, returns the bytes mapped or 0 on failure
static uint64_t heap_grow(size_t size) {
    uint64_t grow = (size + PAGE_SIZE - 1) & ~((uint64_t)PAGE_SIZE - 1);
    if (grow > HEAP_VIRT_MAX - heap_end) {
        return 0;
    }

    for (uint64_t offset = 0; offset < grow; offset += PAGE_SIZE) {
        uint64_t phys = pmm_alloc_page();
        if (phys == 0 || !vmm_map_page(heap_end + offset, phys, PT_WRITABLE)) {
            if (phys != 0) {
                pmm_free_page(phys);
            }
            // undo the pages mapped so far
            for (uint64_t undo = 0; undo < offset; undo += PAGE_SIZE) {
                pmm_free_page(vmm_get_physical(heap_end + undo));
                vmm_unmap_page(heap_end + undo);
            }
            return 0;
        }
    }

    heap_end += grow;
    return grow;
}

int kmalloc_init(void) {
    if (heap_grow(HEAP_INITIAL_SIZE) == 0) {
        serial_writestring("[KMALLOC] Failed to map the initial heap\n");
        return -1;
    }

    heap_start = (block_header_t*)HEAP_VIRT_START;
    heap_start->size = HEAP_INITIAL_SIZE - BLOCK_HEADER_SIZE;
    heap_start->is_free = true;
    heap_start->next = NULL;

    serial_writestring("[KMALLOC] Kernel heap allocator initialized\n");
    return 0;
}

void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    size = align_size(size);

    block_header_t* current = heap_start;
    while (current != NULL) {
        if (current->is_free && current->size >= size) {

            if (current->size >= size + BLOCK_HEADER_SIZE + 16) {
                block_header_t* new_block = (block_header_t*)((uint8_t*)current + BLOCK_HEADER_SIZE + size);
                new_block->size = current->size - size - BLOCK_HEADER_SIZE;
                new_block->is_free = true;
                new_block->next = current->next;

                current->size = size;
                current->next = new_block;
            }

            current->is_free = false;
            total_allocated += current->size;

            return (void*)((uint8_t*)current + BLOCK_HEADER_SIZE);
        }

        current = current->next;
    }

    // no free block fits, so extend the heap and retry
    block_header_t* last = heap_start;
    while (last->next != NULL) {
        last = last->next;
    }

    uint64_t grown = heap_grow(last->is_free ? size - last->size : size + BLOCK_HEADER_SIZE);
    if (grown == 0) {
        return NULL;
    }

    // blocks tile the heap, so the last block always ends where the new pages begin
    if (last->is_free) {
        last->size += grown;
    } else {
        block_header_t* new_block = (block_header_t*)((uint8_t*)last + BLOCK_HEADER_SIZE + last->size);
        new_block->size = grown - BLOCK_HEADER_SIZE;
        new_block->is_free = true;
        new_block->next = NULL;
        last->next = new_block;
    }
    return kmalloc(size);
}

int kfree(void* ptr) {
    if (ptr == NULL) {
        return 0;
    }

    block_header_t* block = (block_header_t*)((uint8_t*)ptr - BLOCK_HEADER_SIZE);

    if (block->is_free) {
        return 0;
    }

    block->is_free = true;
    total_allocated -= block->size;

    if (block->next != NULL && block->next->is_free) {
        block->size += BLOCK_HEADER_SIZE + block->next->size;
        block->next = block->next->next;
    }

    block_header_t* current = heap_start;
    while (current != NULL && current->next != block) {
        current = current->next;
    }

    if (current != NULL && current->is_free) {
        current->size += BLOCK_HEADER_SIZE + block->size;
        current->next = block->next;
    }
    return 0;
}

uint64_t kmalloc_get_used(void) {
    return total_allocated;
}

uint64_t kmalloc_get_free(void) {
    uint64_t free_memory = 0;
    block_header_t* current = heap_start;

    while (current != NULL) {
        if (current->is_free) {
            free_memory += current->size;
        }
        current = current->next;
    }

    return free_memory;
}

int validate_kmalloc(void) {
    uint64_t used_before = kmalloc_get_used();

    // larger than the initial heap, so this has to grow it
    size_t size = 2 * 1024 * 1024;
    uint8_t* buffer = kmalloc(size);
    if (buffer == NULL) {
        serial_writestring("KMALLOC validation failed: could not grow the heap\n");
        return -1;
    }

    // a distinct value per page catches unmapped pages and pages mapped to the same frame
    for (size_t offset = 0; offset < size; offset += PAGE_SIZE) {
        buffer[offset] = (uint8_t)(offset / PAGE_SIZE);
    }
    for (size_t offset = 0; offset < size; offset += PAGE_SIZE) {
        if (buffer[offset] != (uint8_t)(offset / PAGE_SIZE)) {
            serial_writestring("KMALLOC validation failed: heap pages alias each other\n");
            return -1;
        }
    }

    void* small = kmalloc(32);
    if (small == NULL) {
        serial_writestring("KMALLOC validation failed: small allocation after growth\n");
        return -1;
    }
    kfree(small);
    kfree(buffer);

    if (kmalloc_get_used() != used_before) {
        serial_writestring("KMALLOC validation failed: used bytes did not return to baseline\n");
        return -1;
    }

    serial_writestring("KMALLOC validation passed!\n");
    return 0;
}
