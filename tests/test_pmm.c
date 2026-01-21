#include "unity/unity.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Mock serial driver
void serial_writestring(const char* str) {
    // Silent for tests
    (void)str;
}

// Include PMM implementation for testing
#define PAGE_SIZE 4096
#define PAGES_PER_BYTE 8

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
    
    // Allocate bitmap in userspace for testing
    if (page_bitmap != NULL) {
        free(page_bitmap);
    }
    page_bitmap = (uint8_t*)calloc(bitmap_size, 1);
    if (page_bitmap == NULL) {
        printf("FATAL: Failed to allocate page bitmap\n");
        exit(1);
    }

    // Reset tracking
    used_pages = 0;
    
    // Mark some pages as reserved (simplified for testing)
    uint64_t reserved_pages = 256; // Reserve first 1MB (256 * 4KB)
    for (uint64_t i = 0; i < reserved_pages && i < total_pages; i++) {
        set_page_allocated(i);
        used_pages++;
    }
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

// Unity setup and teardown
void setUp(void) {
    pmm_init(8 * 1024 * 1024); // 8MB
}

void tearDown(void) {
    // Cleanup happens in next setUp or at end
}

// Test cases
void test_pmm_init(void) {
    TEST_ASSERT_GREATER_THAN(0, pmm_get_total_pages());
    TEST_ASSERT_GREATER_THAN(0, pmm_get_used_pages());
    TEST_ASSERT_GREATER_THAN(0, pmm_get_free_pages());
}

void test_pmm_alloc_page(void) {
    uint64_t addr = pmm_alloc_page();
    TEST_ASSERT_NOT_EQUAL(0, addr);
    TEST_ASSERT_EQUAL_UINT64(0, addr % PAGE_SIZE);
}

void test_pmm_free_page(void) {
    uint64_t used_before = pmm_get_used_pages();
    uint64_t addr = pmm_alloc_page();
    uint64_t used_after_alloc = pmm_get_used_pages();
    
    TEST_ASSERT_EQUAL_UINT64(used_before + 1, used_after_alloc);
    
    pmm_free_page(addr);
    uint64_t used_after_free = pmm_get_used_pages();
    
    TEST_ASSERT_EQUAL_UINT64(used_before, used_after_free);
}

void test_pmm_multiple_allocations(void) {
    uint64_t addr1 = pmm_alloc_page();
    uint64_t addr2 = pmm_alloc_page();
    uint64_t addr3 = pmm_alloc_page();
    
    TEST_ASSERT_NOT_EQUAL(0, addr1);
    TEST_ASSERT_NOT_EQUAL(0, addr2);
    TEST_ASSERT_NOT_EQUAL(0, addr3);
    
    TEST_ASSERT_NOT_EQUAL(addr1, addr2);
    TEST_ASSERT_NOT_EQUAL(addr2, addr3);
    TEST_ASSERT_NOT_EQUAL(addr1, addr3);
}

void test_pmm_reuse_freed_page(void) {
    // Allocate all free pages except one
    uint64_t free_pages = pmm_get_free_pages();
    uint64_t* addrs = (uint64_t*)malloc(free_pages * sizeof(uint64_t));
    
    for (uint64_t i = 0; i < free_pages - 1; i++) {
        addrs[i] = pmm_alloc_page();
    }
    
    // Now only one page should be free
    TEST_ASSERT_EQUAL_UINT64(1, pmm_get_free_pages());
    
    // Allocate the last page
    uint64_t last_addr = pmm_alloc_page();
    TEST_ASSERT_NOT_EQUAL(0, last_addr);
    TEST_ASSERT_EQUAL_UINT64(0, pmm_get_free_pages());
    
    // Try to allocate when no pages are free
    uint64_t no_page = pmm_alloc_page();
    TEST_ASSERT_EQUAL_UINT64(0, no_page);
    
    // Free one page
    pmm_free_page(addrs[0]);
    TEST_ASSERT_EQUAL_UINT64(1, pmm_get_free_pages());
    
    // Should be able to allocate again
    uint64_t reused = pmm_alloc_page();
    TEST_ASSERT_NOT_EQUAL(0, reused);
    TEST_ASSERT_EQUAL_UINT64(addrs[0], reused);
    
    // Cleanup
    free(addrs);
}

void test_pmm_page_tracking(void) {
    uint64_t total = pmm_get_total_pages();
    uint64_t used = pmm_get_used_pages();
    uint64_t free_mem = pmm_get_free_pages();
    
    TEST_ASSERT_EQUAL_UINT64(total, used + free_mem);
    
    uint64_t addr = pmm_alloc_page();
    
    uint64_t used2 = pmm_get_used_pages();
    uint64_t free2 = pmm_get_free_pages();
    
    TEST_ASSERT_EQUAL_UINT64(used + 1, used2);
    TEST_ASSERT_EQUAL_UINT64(free_mem - 1, free2);
    TEST_ASSERT_EQUAL_UINT64(total, used2 + free2);
    
    pmm_free_page(addr);
}

void test_pmm_invalid_free(void) {
    uint64_t used_before = pmm_get_used_pages();
    
    // Try to free an address beyond the total memory
    uint64_t invalid_addr = (total_pages + 10) * PAGE_SIZE;
    pmm_free_page(invalid_addr);
    
    TEST_ASSERT_EQUAL_UINT64(used_before, pmm_get_used_pages());
}

void test_pmm_double_free(void) {
    uint64_t addr = pmm_alloc_page();
    uint64_t used_after_alloc = pmm_get_used_pages();
    
    pmm_free_page(addr);
    uint64_t used_after_first_free = pmm_get_used_pages();
    
    TEST_ASSERT_EQUAL_UINT64(used_after_alloc - 1, used_after_first_free);
    
    // Try to free again
    pmm_free_page(addr);
    uint64_t used_after_second_free = pmm_get_used_pages();
    
    TEST_ASSERT_EQUAL_UINT64(used_after_first_free, used_after_second_free);
}

void test_pmm_page_alignment(void) {
    for (int i = 0; i < 10; i++) {
        uint64_t addr = pmm_alloc_page();
        TEST_ASSERT_NOT_EQUAL(0, addr);
        TEST_ASSERT_EQUAL_UINT64(0, addr % PAGE_SIZE);
    }
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_pmm_init);
    RUN_TEST(test_pmm_alloc_page);
    RUN_TEST(test_pmm_free_page);
    RUN_TEST(test_pmm_multiple_allocations);
    RUN_TEST(test_pmm_reuse_freed_page);
    RUN_TEST(test_pmm_page_tracking);
    RUN_TEST(test_pmm_invalid_free);
    RUN_TEST(test_pmm_double_free);
    RUN_TEST(test_pmm_page_alignment);
    
    // Cleanup
    if (page_bitmap != NULL) {
        free(page_bitmap);
    }
    
    return UNITY_END();
}
