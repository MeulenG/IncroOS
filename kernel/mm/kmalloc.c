#include "kmalloc.h"
#include "pmm.h"
#include "vmm.h"
#include "../drivers/serial.h"

// Memory block header
typedef struct block_header {
    size_t size;                    // Size of the block (excluding header)
    bool is_free;                   // Whether the block is free
    struct block_header* next;      // Next block in the list
} block_header_t;

// Heap configuration
#define HEAP_START 0x300000         // Start of kernel heap (3MB)
#define HEAP_SIZE  0x100000         // 1MB heap size
#define BLOCK_HEADER_SIZE sizeof(block_header_t)

// Heap state
static block_header_t* heap_start = NULL;
static uint64_t total_allocated = 0;

// Helper: Align size to 16 bytes
static size_t align_size(size_t size) {
    return (size + 15) & ~15;
}

void kmalloc_init(void) {
    // Map heap pages into virtual memory
    uint64_t heap_pages = HEAP_SIZE / PAGE_SIZE;
    for (uint64_t i = 0; i < heap_pages; i++) {
        uint64_t virt = HEAP_START + (i * PAGE_SIZE);
        uint64_t phys = pmm_alloc_page();
        if (phys == 0) {
            serial_writestring("[KMALLOC] Failed to allocate physical page for heap\n");
            return;
        }
        
        if (!vmm_map_page(virt, phys, PT_WRITABLE)) {
            serial_writestring("[KMALLOC] Failed to map heap page\n");
            return;
        }
    }
    
    // Initialize heap with one large free block
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
    
    // Align size
    size = align_size(size);
    
    // Search for a free block
    block_header_t* current = heap_start;
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            // Found a suitable block
            
            // Split block if there's enough space left
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
            
            // Return pointer after header
            return (void*)((uint8_t*)current + BLOCK_HEADER_SIZE);
        }
        
        current = current->next;
    }
    
    // No suitable block found
    return NULL;
}

void kfree(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    
    // Get block header
    block_header_t* block = (block_header_t*)((uint8_t*)ptr - BLOCK_HEADER_SIZE);
    
    if (block->is_free) {
        return;  // Double free
    }
    
    block->is_free = true;
    total_allocated -= block->size;
    
    // Coalesce with next block if it's free
    if (block->next != NULL && block->next->is_free) {
        block->size += BLOCK_HEADER_SIZE + block->next->size;
        block->next = block->next->next;
    }
    
    // Coalesce with previous block if it's free
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
