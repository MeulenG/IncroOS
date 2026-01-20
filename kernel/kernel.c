#include "drivers/serial.h"
#include "output/terminal.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "memory/kmalloc.h"
#include "logger.h"

static void uint64_to_string(uint64_t value, char* buffer) {
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    char temp[32];
    int i = 0;
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}

void kMain(void) {
    serial_init();
    logger_init();

    serial_writestring("\n\n=== IncroOS Kernel Starting ===\n");

    terminal_initialize();

    serial_writestring("===========================================\n");
    serial_writestring("  IncroOS - Kernel Starting\n");
    serial_writestring("===========================================\n");

    terminal_writestring("Hello, 64-bit kernel World!\n");

    LOG_INFO_MSG("Initializing Memory Subsystem...");

    // 4GB
    uint64_t total_memory = 4ULL * 1024 * 1024 * 1024;
    pmm_init(total_memory);

    char buffer[32];
    uint64_to_string(pmm_get_total_pages(), buffer);
    serial_writestring("[PMM] Total pages: ");
    serial_writestring(buffer);
    serial_writestring("\n");

    uint64_to_string(pmm_get_free_pages(), buffer);
    serial_writestring("[PMM] Free pages: ");
    serial_writestring(buffer);
    serial_writestring("\n");

    uint64_to_string(pmm_get_used_pages(), buffer);
    serial_writestring("[PMM] Used pages: ");
    serial_writestring(buffer);
    serial_writestring("\n");

    vmm_init();

    kmalloc_init();

    LOG_INFO_MSG("Memory Subsystem Initialized Successfully");

    LOG_INFO_MSG("Testing Memory Allocation...");

    void* ptr1 = kmalloc(64);
    if (ptr1) {
        LOG_DEBUG_MSG("kmalloc(64) succeeded");
        kfree(ptr1);
        LOG_DEBUG_MSG("kfree(64) succeeded");
    } else {
        LOG_ERROR_MSG("kmalloc(64) failed");
    }

    void* ptr2 = kmalloc(1024);
    if (ptr2) {
        LOG_DEBUG_MSG("kmalloc(1024) succeeded");
        kfree(ptr2);
        LOG_DEBUG_MSG("kfree(1024) succeeded");
    } else {
        LOG_ERROR_MSG("kmalloc(1024) failed");
    }

    uint64_t page = pmm_alloc_page();
    if (page) {
        LOG_DEBUG_MSG("pmm_alloc_page() succeeded");
        pmm_free_page(page);
        LOG_DEBUG_MSG("pmm_free_page() succeeded");
    } else {
        LOG_ERROR_MSG("pmm_alloc_page() failed");
    }

    uint64_to_string(kmalloc_get_used(), buffer);
    serial_writestring("\n[HEAP] Used memory: ");
    serial_writestring(buffer);
    serial_writestring(" bytes\n");

    uint64_to_string(kmalloc_get_free(), buffer);
    serial_writestring("[HEAP] Free memory: ");
    serial_writestring(buffer);
    serial_writestring(" bytes\n");

    serial_writestring("\n===========================================\n");
    serial_writestring("  Memory Manager Tests Complete\n");
    serial_writestring("===========================================\n");

    terminal_writestring("Memory Manager Initialized!\n");

    while (1) {
        __asm__ volatile("hlt");
    }
}
