#include "drivers/serial.h"
#include "output/terminal.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "memory/kmalloc.h"

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

    serial_writestring("\n\n=== IncroOS Kernel Starting ===\n");

    terminal_initialize();

    serial_writestring("===========================================\n");
    serial_writestring("  IncroOS - Kernel Starting\n");
    serial_writestring("===========================================\n");

    terminal_writestring("Hello, 64-bit kernel World!\n");

    serial_writestring("\n[INIT] Initializing Memory Subsystem...\n");

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

    serial_writestring("\n[INIT] Memory Subsystem Initialized Successfully\n");

    serial_writestring("\n[TEST] Testing Memory Allocation...\n");

    void* ptr1 = kmalloc(64);
    if (ptr1) {
        serial_writestring("[TEST] kmalloc(64) succeeded\n");
        kfree(ptr1);
        serial_writestring("[TEST] kfree(64) succeeded\n");
    } else {
        serial_writestring("[TEST] kmalloc(64) failed\n");
    }

    void* ptr2 = kmalloc(1024);
    if (ptr2) {
        serial_writestring("[TEST] kmalloc(1024) succeeded\n");
        kfree(ptr2);
        serial_writestring("[TEST] kfree(1024) succeeded\n");
    } else {
        serial_writestring("[TEST] kmalloc(1024) failed\n");
    }

    uint64_t page = pmm_alloc_page();
    if (page) {
        serial_writestring("[TEST] pmm_alloc_page() succeeded, page at: 0x");
        for (int i = 60; i >= 0; i -= 4) {
            uint8_t nibble = (page >> i) & 0xF;
            char hex = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
            char hex_str[2] = {hex, '\0'};
            serial_writestring(hex_str);
        }
        serial_writestring("\n");
        pmm_free_page(page);
        serial_writestring("[TEST] pmm_free_page() succeeded\n");
    } else {
        serial_writestring("[TEST] pmm_alloc_page() failed\n");
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
