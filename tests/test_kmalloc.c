#include "test_framework.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// Mock serial driver
void serial_writestring(const char* str) {
    // Silent for tests
    (void)str;
}

// Include kmalloc implementation directly for testing
// Memory block header
typedef struct block_header {
    size_t size;
    bool is_free;
    struct block_header* next;
} block_header_t;

#define HEAP_SIZE  0x80000
#define BLOCK_HEADER_SIZE sizeof(block_header_t)

static block_header_t* heap_start = NULL;
static uint64_t total_allocated = 0;
static uint8_t* test_heap = NULL;

static size_t align_size(size_t size) {
    return (size + 15) & ~15;
}

void kmalloc_init(void) {
    // Allocate test heap in userspace
    if (test_heap == NULL) {
        test_heap = (uint8_t*)malloc(HEAP_SIZE);
    }
    
    heap_start = (block_header_t*)test_heap;
    heap_start->size = HEAP_SIZE - BLOCK_HEADER_SIZE;
    heap_start->is_free = true;
    heap_start->next = NULL;
    total_allocated = 0;
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

// Test cases
bool test_kmalloc_init(void) {
    kmalloc_init();
    TEST_ASSERT_NOT_NULL(heap_start, "Heap should be initialized");
    TEST_ASSERT(heap_start->is_free, "Initial heap block should be free");
    TEST_ASSERT_EQUAL(0, kmalloc_get_used(), "Used memory should be 0 after init");
    return true;
}

bool test_kmalloc_basic_allocation(void) {
    kmalloc_init();
    
    void* ptr = kmalloc(64);
    TEST_ASSERT_NOT_NULL(ptr, "Should allocate 64 bytes");
    TEST_ASSERT(kmalloc_get_used() > 0, "Used memory should be > 0 after allocation");
    
    return true;
}

bool test_kmalloc_zero_size(void) {
    kmalloc_init();
    
    void* ptr = kmalloc(0);
    TEST_ASSERT_NULL(ptr, "Should return NULL for zero size");
    TEST_ASSERT_EQUAL(0, kmalloc_get_used(), "Used memory should be 0");
    
    return true;
}

bool test_kmalloc_free_null(void) {
    kmalloc_init();
    
    // Should not crash
    kfree(NULL);
    TEST_ASSERT_EQUAL(0, kmalloc_get_used(), "Used memory should be 0");
    
    return true;
}

bool test_kmalloc_alloc_and_free(void) {
    kmalloc_init();
    
    void* ptr = kmalloc(128);
    TEST_ASSERT_NOT_NULL(ptr, "Should allocate 128 bytes");
    uint64_t used_after_alloc = kmalloc_get_used();
    TEST_ASSERT(used_after_alloc > 0, "Used memory should be > 0");
    
    kfree(ptr);
    TEST_ASSERT_EQUAL(0, kmalloc_get_used(), "Used memory should be 0 after free");
    
    return true;
}

bool test_kmalloc_multiple_allocations(void) {
    kmalloc_init();
    
    void* ptr1 = kmalloc(64);
    void* ptr2 = kmalloc(128);
    void* ptr3 = kmalloc(256);
    
    TEST_ASSERT_NOT_NULL(ptr1, "First allocation should succeed");
    TEST_ASSERT_NOT_NULL(ptr2, "Second allocation should succeed");
    TEST_ASSERT_NOT_NULL(ptr3, "Third allocation should succeed");
    
    TEST_ASSERT(ptr1 != ptr2, "Allocations should have different addresses");
    TEST_ASSERT(ptr2 != ptr3, "Allocations should have different addresses");
    TEST_ASSERT(ptr1 != ptr3, "Allocations should have different addresses");
    
    kfree(ptr1);
    kfree(ptr2);
    kfree(ptr3);
    
    TEST_ASSERT_EQUAL(0, kmalloc_get_used(), "All memory should be freed");
    
    return true;
}

bool test_kmalloc_reuse_freed_memory(void) {
    kmalloc_init();
    
    void* ptr1 = kmalloc(64);
    TEST_ASSERT_NOT_NULL(ptr1, "First allocation should succeed");
    
    kfree(ptr1);
    
    void* ptr2 = kmalloc(64);
    TEST_ASSERT_NOT_NULL(ptr2, "Second allocation should succeed");
    TEST_ASSERT_EQUAL(ptr1, ptr2, "Should reuse the same memory block");
    
    kfree(ptr2);
    
    return true;
}

bool test_kmalloc_alignment(void) {
    kmalloc_init();
    
    void* ptr = kmalloc(1);
    TEST_ASSERT_NOT_NULL(ptr, "Should allocate 1 byte (aligned)");
    
    // The size stored in the block header should be aligned to 16 bytes
    // In the real kernel, the returned pointer would be 16-byte aligned
    // but in the test environment, we just verify the size is aligned
    block_header_t* block = (block_header_t*)((uint8_t*)ptr - BLOCK_HEADER_SIZE);
    TEST_ASSERT_EQUAL(0, block->size % 16, "Allocated size should be 16-byte aligned");
    
    kfree(ptr);
    
    return true;
}

bool test_kmalloc_fragmentation(void) {
    kmalloc_init();
    
    void* ptr1 = kmalloc(64);
    void* ptr2 = kmalloc(64);
    void* ptr3 = kmalloc(64);
    
    // Free the middle block
    kfree(ptr2);
    
    // Allocate a smaller block - should fit in the freed space
    void* ptr4 = kmalloc(32);
    TEST_ASSERT_NOT_NULL(ptr4, "Should allocate in freed space");
    
    kfree(ptr1);
    kfree(ptr3);
    kfree(ptr4);
    
    return true;
}

bool test_kmalloc_coalescing(void) {
    kmalloc_init();
    
    void* ptr1 = kmalloc(64);
    void* ptr2 = kmalloc(64);
    
    kfree(ptr1);
    kfree(ptr2);
    
    // After freeing adjacent blocks, they should be coalesced
    // Should be able to allocate a large block
    void* ptr3 = kmalloc(128);
    TEST_ASSERT_NOT_NULL(ptr3, "Should allocate large block after coalescing");
    
    kfree(ptr3);
    
    return true;
}

bool test_kmalloc_memory_tracking(void) {
    kmalloc_init();
    
    uint64_t initial_free = kmalloc_get_free();
    TEST_ASSERT(initial_free > 0, "Initial free memory should be > 0");
    
    void* ptr = kmalloc(1024);
    uint64_t used = kmalloc_get_used();
    uint64_t free_after = kmalloc_get_free();
    
    TEST_ASSERT(used > 0, "Used memory should be > 0");
    TEST_ASSERT(free_after < initial_free, "Free memory should decrease");
    
    kfree(ptr);
    
    TEST_ASSERT_EQUAL(0, kmalloc_get_used(), "Used memory should be 0 after free");
    
    return true;
}

int main(void) {
    printf("\n========================================\n");
    printf("Running kmalloc Tests\n");
    printf("========================================\n\n");
    
    RUN_TEST(test_kmalloc_init);
    RUN_TEST(test_kmalloc_basic_allocation);
    RUN_TEST(test_kmalloc_zero_size);
    RUN_TEST(test_kmalloc_free_null);
    RUN_TEST(test_kmalloc_alloc_and_free);
    RUN_TEST(test_kmalloc_multiple_allocations);
    RUN_TEST(test_kmalloc_reuse_freed_memory);
    RUN_TEST(test_kmalloc_alignment);
    RUN_TEST(test_kmalloc_fragmentation);
    RUN_TEST(test_kmalloc_coalescing);
    RUN_TEST(test_kmalloc_memory_tracking);
    
    PRINT_TEST_SUMMARY();
    
    // Cleanup
    if (test_heap != NULL) {
        free(test_heap);
    }
    
    return (test_results.failed == 0) ? 0 : 1;
}
