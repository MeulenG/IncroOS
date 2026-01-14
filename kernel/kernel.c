#include "drivers/serial.h"
#include "output/terminal.h"

<<<<<<< HEAD
=======
// Buffer size for uint64_t to string conversion
// Maximum 20 digits for uint64_t + 1 null terminator
#define UINT64_STR_BUFFER_SIZE 21

// Helper function to convert uint64_t to string.
// Caller must provide a buffer of at least UINT64_STR_BUFFER_SIZE bytes (21 bytes).
static void uint64_to_str(uint64_t value, char* buffer) {
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    int i = 0;
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    buffer[i] = '\0';
    
    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

>>>>>>> 1e0d697 (Address code review feedback: improve variable naming, optimize memory operations, add documentation)
void kMain(void) {
    serial_init();
    terminal_initialize();

    terminal_writestring("Hello, 64-bit kernel World!\n");
<<<<<<< HEAD
=======
    terminal_writestring("Initializing Memory Manager...\n");
    
    // Initialize memory manager (assume 128MB for now)
    pmm_init(128 * 1024 * 1024);
    vmm_init();
    heap_init();
    
    terminal_writestring("Memory Manager Initialized!\n\n");
    
    // Display memory info
    terminal_writestring("Memory Statistics:\n");
    
    char buffer[UINT64_STR_BUFFER_SIZE];
    uint64_to_str(pmm_get_total_memory() / 1024 / 1024, buffer);
    terminal_writestring("  Total Memory: ");
    terminal_writestring(buffer);
    terminal_writestring(" MB\n");
    
    uint64_to_str(pmm_get_used_memory() / 1024, buffer);
    terminal_writestring("  Used Memory: ");
    terminal_writestring(buffer);
    terminal_writestring(" KB\n");
    
    uint64_to_str(pmm_get_free_memory() / 1024 / 1024, buffer);
    terminal_writestring("  Free Memory: ");
    terminal_writestring(buffer);
    terminal_writestring(" MB\n\n");
    
    // Test physical memory allocator
    terminal_writestring("Testing Physical Memory Allocator...\n");
    uint64_t page1 = pmm_alloc_page();
    uint64_to_str(page1, buffer);
    terminal_writestring("  Allocated page at: 0x");
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    uint64_t page2 = pmm_alloc_page();
    uint64_to_str(page2, buffer);
    terminal_writestring("  Allocated page at: 0x");
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    pmm_free_page(page1);
    terminal_writestring("  Freed first page\n\n");
    
    // Test heap allocator
    terminal_writestring("Testing Heap Allocator...\n");
    void* ptr1 = kmalloc(100);
    terminal_writestring("  kmalloc(100) = 0x");
    uint64_to_str((uint64_t)ptr1, buffer);
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    void* ptr2 = kmalloc(256);
    terminal_writestring("  kmalloc(256) = 0x");
    uint64_to_str((uint64_t)ptr2, buffer);
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    kfree(ptr1);
    terminal_writestring("  Freed first allocation\n");
    
    void* ptr3 = kmalloc(50);
    terminal_writestring("  kmalloc(50) = 0x");
    uint64_to_str((uint64_t)ptr3, buffer);
    terminal_writestring(buffer);
    terminal_writestring("\n\n");
    
    terminal_writestring("Memory Manager Tests Complete!\n");
>>>>>>> 1e0d697 (Address code review feedback: improve variable naming, optimize memory operations, add documentation)
}