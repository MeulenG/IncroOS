#include "heap.h"
#include "vmm.h"
#include "pmm.h"

// Simple heap implementation using a linked list of free blocks
// This is a basic first-fit allocator

typedef struct heap_block {
    size_t size;                    // Size of the block (excluding header)
    bool is_free;                   // Whether the block is free
    struct heap_block* next;        // Next block in the list
} heap_block_t;

// Heap configuration
#define HEAP_START      0x10000000  // Start at 256MB virtual address
#define HEAP_INITIAL    (1024 * 1024)  // Start with 1MB heap
#define HEAP_MAX        (16 * 1024 * 1024)  // Maximum 16MB heap

static heap_block_t* heap_head = 0;
static uint64_t heap_current_size = 0;
static uint64_t heap_current_end = HEAP_START;

// Helper structure for heap expansion
typedef struct {
    uint64_t addr;
    size_t size;
} heap_expand_result_t;

// Helper function to expand heap
// Returns the start address and size of the newly allocated region
static heap_expand_result_t expand_heap(size_t additional_size) {
    heap_expand_result_t result = {0, 0};
    
    // Align to page boundary
    size_t pages_needed = (additional_size + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t new_size = pages_needed * PAGE_SIZE;
    
    if (heap_current_size + new_size > HEAP_MAX) {
        return result;  // Would exceed maximum heap size
    }
    
    // Save start of new region
    uint64_t new_region_start = heap_current_end;
    
    // Allocate and map pages
    for (size_t i = 0; i < pages_needed; i++) {
        uint64_t phys = pmm_alloc_page();
        if (phys == 0) {
            return result;  // Out of physical memory
        }
        
        uint64_t virt = heap_current_end + (i * PAGE_SIZE);
        if (!vmm_map_page(virt, phys, PTE_PRESENT | PTE_WRITABLE)) {
            pmm_free_page(phys);
            return result;  // Mapping failed
        }
    }
    
    heap_current_end += new_size;
    heap_current_size += new_size;
    
    result.addr = new_region_start;
    result.size = new_size;
    return result;
}

void heap_init(void) {
    // Expand heap to initial size
    heap_expand_result_t result = expand_heap(HEAP_INITIAL);
    if (result.addr == 0) {
        return;  // Failed to initialize heap
    }
    
    // Create initial free block
    heap_head = (heap_block_t*)result.addr;
    heap_head->size = result.size - sizeof(heap_block_t);
    heap_head->is_free = true;
    heap_head->next = 0;
}

void* kmalloc(size_t size) {
    if (size == 0) {
        return 0;
    }
    
    // Align size to 8 bytes
    size = (size + 7) & ~7;
    
    // Find a suitable free block
    heap_block_t* current = heap_head;
    heap_block_t* prev = 0;
    
    while (current) {
        if (current->is_free && current->size >= size) {
            // Found a suitable block
            
            // If block is much larger, split it
            if (current->size >= size + sizeof(heap_block_t) + 64) {
                heap_block_t* new_block = (heap_block_t*)((uint8_t*)current + sizeof(heap_block_t) + size);
                new_block->size = current->size - size - sizeof(heap_block_t);
                new_block->is_free = true;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->is_free = false;
            return (void*)((uint8_t*)current + sizeof(heap_block_t));
        }
        
        prev = current;
        current = current->next;
    }
    
    // No suitable block found, try to expand heap
    size_t needed = size + sizeof(heap_block_t);
    heap_expand_result_t result = expand_heap(needed);
    if (result.addr != 0) {
        // Create new block at start of new region
        heap_block_t* new_block = (heap_block_t*)result.addr;
        new_block->size = result.size - sizeof(heap_block_t);
        new_block->is_free = false;
        new_block->next = 0;
        
        // If block is much larger than needed, split it
        if (new_block->size >= size + sizeof(heap_block_t) + 64) {
            heap_block_t* free_block = (heap_block_t*)((uint8_t*)new_block + sizeof(heap_block_t) + size);
            free_block->size = new_block->size - size - sizeof(heap_block_t);
            free_block->is_free = true;
            free_block->next = 0;
            
            new_block->size = size;
            new_block->next = free_block;
        }
        
        // Link to end of list
        if (prev) {
            prev->next = new_block;
        } else {
            heap_head = new_block;
        }
        
        return (void*)((uint8_t*)new_block + sizeof(heap_block_t));
    }
    
    return 0;  // Out of memory
}

void kfree(void* ptr) {
    if (!ptr) {
        return;
    }
    
    // Get block header
    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    block->is_free = true;
    
    // Coalesce adjacent free blocks
    heap_block_t* current = heap_head;
    while (current && current->next) {
        if (current->is_free && current->next->is_free) {
            current->size += sizeof(heap_block_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

size_t heap_get_used(void) {
    size_t used = 0;
    heap_block_t* current = heap_head;
    
    while (current) {
        if (!current->is_free) {
            used += current->size + sizeof(heap_block_t);
        }
        current = current->next;
    }
    
    return used;
}

size_t heap_get_free(void) {
    return heap_current_size - heap_get_used();
}
