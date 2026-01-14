#include "drivers/serial.h"
#include "output/terminal.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "mm/heap.h"

// Simple helper to convert number to string
static void uint_to_string(uint64_t value, char* buffer) {
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
    terminal_initialize();

    terminal_writestring("Hello, 64-bit kernel World!\n");
    
    // Initialize memory management subsystems
    terminal_writestring("Initializing memory manager...\n");
    
    // Initialize physical memory manager
    pmm_init();
    terminal_writestring("PMM initialized\n");
    
    // Initialize virtual memory manager
    vmm_init();
    terminal_writestring("VMM initialized\n");
    
    // Initialize heap allocator
    heap_init();
    terminal_writestring("Heap initialized\n");
    
    // Display memory statistics
    char buffer[32];
    
    terminal_writestring("Total pages: ");
    uint_to_string(pmm_get_total_pages(), buffer);
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    terminal_writestring("Free pages: ");
    uint_to_string(pmm_get_free_pages(), buffer);
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    // Test memory allocation
    terminal_writestring("\nTesting memory allocation...\n");
    
    // Test kmalloc
    void* ptr1 = kmalloc(100);
    if (ptr1) {
        terminal_writestring("kmalloc(100): SUCCESS\n");
        kfree(ptr1);
        terminal_writestring("kfree: SUCCESS\n");
    } else {
        terminal_writestring("kmalloc(100): FAILED\n");
    }
    
    // Test page allocation
    uint64_t page = pmm_alloc_page();
    if (page) {
        terminal_writestring("pmm_alloc_page: SUCCESS (addr: 0x");
        // Simple hex conversion for address
        char hex_buffer[17];
        for (int i = 15; i >= 0; i--) {
            uint8_t nibble = (page >> (i * 4)) & 0xF;
            hex_buffer[15 - i] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
        }
        hex_buffer[16] = '\0';
        terminal_writestring(hex_buffer);
        terminal_writestring(")\n");
        pmm_free_page(page);
        terminal_writestring("pmm_free_page: SUCCESS\n");
    } else {
        terminal_writestring("pmm_alloc_page: FAILED\n");
    }
    
    terminal_writestring("\nMemory manager operational!\n");
}