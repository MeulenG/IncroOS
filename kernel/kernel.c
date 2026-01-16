#include "drivers/serial.h"
#include "output/terminal.h"
#include "interrupts/interrupts.h"

// Enable interrupts
static inline void sti(void) {
    __asm__ volatile("sti");
}

void kMain(void) {
    serial_init();
    terminal_initialize();

    terminal_writestring("Hello, 64-bit kernel World!\n");
    terminal_writestring("Initializing IDT...\n");
    
    // Initialize interrupt handling
    idt_init();
    
    terminal_writestring("IDT initialized\n");
    terminal_writestring("Initializing PIT timer...\n");
    
    // Initialize PIT to generate interrupts at ~18.222 Hz (default DOS rate)
    pit_init(18);
    
    terminal_writestring("PIT initialized\n");
    terminal_writestring("Enabling interrupts...\n");
    
    // Enable interrupts
    sti();
    
    terminal_writestring("Interrupts enabled! Timer is running.\n");
    
    // Infinite loop - let interrupts do their work
    while(1) {
        __asm__ volatile("hlt"); // Halt until next interrupt
    }
}