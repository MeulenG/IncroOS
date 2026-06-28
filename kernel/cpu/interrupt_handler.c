#include "interrupt_handler.h"


// Logging for interrupts
void log_interrupt(const char* interrupt_name) {
    serial_writestring("[INTERRUPT] ");
    serial_writestring(interrupt_name);
    serial_writestring(" interrupt fired!\n");

    terminal_writestring("[INTERRUPT] ");
    terminal_writestring(interrupt_name);
    terminal_writestring(" interrupt fired!\n");
}

// The interrupt attribute tells the compiler that, the iret instruction will be used instead of ret on x86-64
// This allows the handler to receive a pointer to an interrupt_frame struct containing the CPU state at the time of the interrupt
__attribute__((interrupt)) void isr_divide_by_zero(struct interrupt_frame* frame) {
    log_interrupt("Divide by Zero");
    for(;;);
}

__attribute__((interrupt)) void isr_page_fault(struct interrupt_frame* frame, uintptr_t error_code) {
    log_interrupt("Page Fault");
    for(;;);
}

__attribute__((interrupt)) void irq0_handler(struct interrupt_frame* frame) {
    serial_writestring("IRQ0: Timer interrupt received\n");
    // Send an EOI to the PICs
    PIC_sendEOI(0);
}

// Read the keyboard scancode and send an EOI to the PICs
__attribute__((interrupt)) void irq1_handler(struct interrupt_frame* frame) {
    // read from port 0x60 to acknowledge the keyboard interrupt
    // For now: if the scancode is above 0x80, return early as it is a key release event and we are not handling those yet
    uint8_t scancode = inb(0x60);
    if (scancode & 0x80) {
        PIC_sendEOI(1);
        return;
    }
    // kprintf("IRQ1: Keyboard interrupt received, scancode: 0x%x\n", scancode);
    // parse it to the keyboard driver
    handle_keyboard_interrupt(scancode);
    // Send an EOI to the PICs
    PIC_sendEOI(1);
}

// Spurious IRQ for IRQ7 and IRQ15
__attribute__((interrupt)) void irq7_handler(struct interrupt_frame* frame) {
    serial_writestring("IRQ7: Spurious interrupt received\n");
    // Dont send an EOI to the PICs for spurious interrupts, as it can cause issues
    // we just log it for now since we are in early development
}

__attribute__((interrupt)) void irq15_handler(struct interrupt_frame* frame) {
    serial_writestring("IRQ15: Spurious interrupt received\n");
    // Dont send an EOI to the PICs for spurious interrupts, as it can cause issues
    // we just log it for now since we are in early development
}
