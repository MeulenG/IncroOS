#include "boot_service.h"
#include "vesa_terminal.h"


// takes an array of structs boot_service and iterates through them, calling the function pointer for each service
void run_boot_services(struct boot_service* services, int count) {
    // Initialize the terminal
    vesa_terminal_initialize();
    vesa_terminal_writestring("Boot service started...\n", VESA_COLOR_LIGHT_GREY);
    // Call the provided function pointer
    for (int i = 0; i < count; i++) {
        struct boot_service* service = &services[i];
        int result = service->fptr();
        if (result != 0) {
            vesa_terminal_writestring("Boot service failed: ", VESA_COLOR_LIGHT_RED);
            vesa_terminal_writestring(service->name, VESA_COLOR_LIGHT_RED);
            vesa_terminal_writestring("\n", VESA_COLOR_LIGHT_RED);
            if (service->isCritical) {
                vesa_terminal_writestring("Critical boot service failed. Halting system.\n", VESA_COLOR_LIGHT_RED);
                for(;;);
            }
        }
        // only validate a service that initialized, and skip services without a validator
        if (result == 0 && service->validate != NULL) {
            int test_result = service->validate();
            if (test_result != 0) {
                vesa_terminal_writestring("Boot service validation failed: ", VESA_COLOR_LIGHT_RED);
                vesa_terminal_writestring(service->name, VESA_COLOR_LIGHT_RED);
                vesa_terminal_writestring("\n", VESA_COLOR_LIGHT_RED);
                if (service->isCritical) {
                    vesa_terminal_writestring("Critical boot service validation failed. Halting system.\n", VESA_COLOR_LIGHT_RED);
                    for(;;);
                }
            }
        }
        // Write to terminal that the service has completed
        vesa_terminal_writestring("[   ", VESA_COLOR_LIGHT_GREY);
        if (result == 0) {
            vesa_terminal_writestring("OK", VESA_COLOR_LIGHT_GREEN);
            vesa_terminal_writestring("   ]", VESA_COLOR_LIGHT_GREY);
            vesa_terminal_writestring(" Boot service completed: ", VESA_COLOR_LIGHT_GREY);
        } else {
            vesa_terminal_writestring("FAIL", VESA_COLOR_LIGHT_RED);
            vesa_terminal_writestring("   ]", VESA_COLOR_LIGHT_GREY);
            vesa_terminal_writestring(" Boot service failed: ", VESA_COLOR_LIGHT_RED);
        }
        vesa_terminal_writestring(service->name, VESA_COLOR_LIGHT_GREY);
        vesa_terminal_writestring("\n", VESA_COLOR_LIGHT_GREY);
    }
}