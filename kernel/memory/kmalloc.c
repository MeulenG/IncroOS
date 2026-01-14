#include "kmalloc.h"
#include "vmm.h"
#include "pmm.h"

// Simple heap implementation using first-fit allocation
// Heap starts at 2MB (after kernel) and grows upward

#define HEAP_START 0x200000
#define HEAP_MAX_SIZE (32 * 1024 * 1024)  // 32MB max heap

// Memory block header
typedef struct mem_block {
    size_t size;              // Size of the block (excluding header)
    bool is_free;            // Whether the block is free
    struct mem_block* next;   // Next block in the list
} mem_block_t;

static mem_block_t* heap_start = NULL;
static uint64_t heap_end = HEAP_START;

// Helper function to set memory to a value (like memset)
static void kmemset(void* ptr, int value, size_t size) {
    uint8_t* bytes = (uint8_t*)ptr;
    uint8_t byte_value = (uint8_t)value;
    
    // Align to word boundary for optimization
    while (size > 0 && ((uint64_t)bytes & 7)) {
        *bytes++ = byte_value;
        size--;
    }
    
    // Fill 8 bytes at a time
    if (size >= 8) {
        uint64_t word_value = byte_value;
        word_value |= word_value << 8;
        word_value |= word_value << 16;
        word_value |= word_value << 32;
        
        uint64_t* words = (uint64_t*)bytes;
        while (size >= 8) {
            *words++ = word_value;
            size -= 8;
        }
        bytes = (uint8_t*)words;
    }
    
    // Fill remaining bytes
    while (size > 0) {
        *bytes++ = byte_value;
        size--;
    }
}

// Helper function to copy memory (like memcpy)
static void kmemcpy(void* dst, const void* src, size_t size) {
    uint8_t* d = (uint8_t*)dst;
    const uint8_t* s = (const uint8_t*)src;
    
    // Align to word boundary for optimization
    while (size > 0 && ((uint64_t)d & 7)) {
        *d++ = *s++;
        size--;
    }
    
    // Copy 8 bytes at a time
    if (size >= 8 && !((uint64_t)s & 7)) {
        uint64_t* dwords = (uint64_t*)d;
        const uint64_t* swords = (const uint64_t*)s;
        while (size >= 8) {
            *dwords++ = *swords++;
            size -= 8;
        }
        d = (uint8_t*)dwords;
        s = (const uint8_t*)swords;
    }
    
    // Copy remaining bytes
    while (size > 0) {
        *d++ = *s++;
        size--;
    }
}

// Helper to align size to 16 bytes
static size_t align_size(size_t size) {
    return (size + 15) & ~15;
}

void heap_init(void) {
    heap_start = (mem_block_t*)HEAP_START;
    heap_start->size = 0;
    heap_start->is_free = true;
    heap_start->next = NULL;
    heap_end = HEAP_START + sizeof(mem_block_t);
}

// Expand heap by allocating more pages
static bool expand_heap(size_t required_size) {
    size_t size_needed = align_size(required_size + sizeof(mem_block_t));
    size_t pages_needed = (size_needed + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // Check if we're within heap limits
    if (heap_end + (pages_needed * PAGE_SIZE) > HEAP_START + HEAP_MAX_SIZE) {
        return false;
    }
    
    // Allocate and map pages
    for (size_t i = 0; i < pages_needed; i++) {
        uint64_t virt_addr = heap_end + (i * PAGE_SIZE);
        if (!vmm_alloc_page(virt_addr, PTE_WRITABLE)) {
            return false;
        }
    }
    
    heap_end += pages_needed * PAGE_SIZE;
    return true;
}

void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    size = align_size(size);
    mem_block_t* current = heap_start;
    
    // First-fit allocation
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            // Found a suitable block
            if (current->size > size + sizeof(mem_block_t) + 16) {
                // Split the block
                mem_block_t* new_block = (mem_block_t*)((uint8_t*)current + sizeof(mem_block_t) + size);
                new_block->size = current->size - size - sizeof(mem_block_t);
                new_block->is_free = true;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->is_free = false;
            return (void*)((uint8_t*)current + sizeof(mem_block_t));
        }
        
        // Move to next block
        if (current->next == NULL) {
            // Need to expand heap
            uint64_t new_block_addr = (uint64_t)current + sizeof(mem_block_t) + current->size;
            
            // Check if we need more pages
            if (new_block_addr + size + sizeof(mem_block_t) > heap_end) {
                if (!expand_heap(size)) {
                    return NULL;
                }
            }
            
            // Create new block
            mem_block_t* new_block = (mem_block_t*)new_block_addr;
            new_block->size = size;
            new_block->is_free = false;
            new_block->next = NULL;
            current->next = new_block;
            
            return (void*)((uint8_t*)new_block + sizeof(mem_block_t));
        }
        
        current = current->next;
    }
    
    return NULL;
}

void kfree(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    
    mem_block_t* block = (mem_block_t*)((uint8_t*)ptr - sizeof(mem_block_t));
    block->is_free = true;
    
    // Coalesce with next block if it's free
    if (block->next != NULL && block->next->is_free) {
        block->size += sizeof(mem_block_t) + block->next->size;
        block->next = block->next->next;
    }
    
    // Coalesce with previous block if it's free
    mem_block_t* current = heap_start;
    while (current != NULL && current->next != block) {
        current = current->next;
    }
    
    if (current != NULL && current->is_free) {
        current->size += sizeof(mem_block_t) + block->size;
        current->next = block->next;
    }
}

void* kzalloc(size_t size) {
    void* ptr = kmalloc(size);
    if (ptr != NULL) {
        kmemset(ptr, 0, size);
    }
    return ptr;
}

void* krealloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return kmalloc(size);
    }
    
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    mem_block_t* block = (mem_block_t*)((uint8_t*)ptr - sizeof(mem_block_t));
    
    if (block->size >= size) {
        return ptr;  // Current block is large enough
    }
    
    // Allocate new block and copy data
    void* new_ptr = kmalloc(size);
    if (new_ptr != NULL) {
        size_t copy_size = (block->size < size) ? block->size : size;
        kmemcpy(new_ptr, ptr, copy_size);
        kfree(ptr);
    }
    
    return new_ptr;
}
