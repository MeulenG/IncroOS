#include "interrupt_handler.h"
#include "../drivers/serial.h"
#include "../output/terminal.h"


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