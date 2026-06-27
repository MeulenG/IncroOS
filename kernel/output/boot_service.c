#include "boot_service.h"
#include "terminal.h"


// takes an array of structs boot_service and iterates through them, calling the function pointer for each service
void run_boot_services(struct boot_service* services, int count) {
    // Initialize the terminal
    terminal_initialize();
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("Boot service started...\n");
    // Call the provided function pointer
    for (int i = 0; i < count; i++) {
        struct boot_service* service = &services[i];
        int result = service->fptr();
        if (result != 0) {
            terminal_writestring("Boot service failed: ");
            terminal_writestring(service->name);
            terminal_writestring("\n");
            if (service->isCritical) {
                terminal_writestring("Critical boot service failed. Halting system.\n");
                for(;;);
            }
        }
        // Write to terminal that the service has completed
        terminal_writestring("[   ");
        if (result == 0) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            terminal_writestring("OK");
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
            terminal_writestring("   ]");
            terminal_writestring(" Boot service completed: ");
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("FAIL");
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
            terminal_writestring("   ]");
            terminal_writestring(" Boot service failed: ");
        }
        terminal_writestring(service->name);
        terminal_writestring("\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    }
}