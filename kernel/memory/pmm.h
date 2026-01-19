#ifndef __PMM_H__
#define __PMM_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// 4KB pages
#define PAGE_SIZE 4096
#define PAGES_PER_BYTE 8

void pmm_init(uint64_t total_memory);
void pmm_free_page(uint64_t addr);

uint64_t pmm_alloc_page(void);
uint64_t pmm_get_total_pages(void);
uint64_t pmm_get_free_pages(void);
uint64_t pmm_get_used_pages(void);

#endif // __PMM_H__
