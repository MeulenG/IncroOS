#include "kmalloc.h"
#include "../drivers/serial.h"
#include <stdbool.h>

// Memory block header
typedef struct block_header {
    size_t size;
    bool is_free;
    struct block_header* next;
} block_header_t;

#define HEAP_START 0x180000
#define HEAP_SIZE  0x80000
#define BLOCK_HEADER_SIZE sizeof(block_header_t)

static block_header_t* heap_start = NULL;
static uint64_t total_allocated = 0;

static size_t align_size(size_t size) {
    return (size + 15) & ~15;
}

void kmalloc_init(void) {
    heap_start = (block_header_t*)HEAP_START;
    heap_start->size = HEAP_SIZE - BLOCK_HEADER_SIZE;
    heap_start->is_free = true;
    heap_start->next = NULL;

    serial_writestring("[KMALLOC] Kernel heap allocator initialized\n");
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

    return NULL;
}

void kfree(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    block_header_t* block = (block_header_t*)((uint8_t*)ptr - BLOCK_HEADER_SIZE);

    if (block->is_free) {
        return;
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
