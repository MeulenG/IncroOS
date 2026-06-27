#include "idt.h"
#include "interrupt_handler.h"
#include "pic.h"

struct InterruptDescriptor64 idt[256];

void unhandled_interrupt() {
    serial_writestring("unhandled interrupt fired!\n");
    terminal_writestring("unhandled interrupt fired!\n");
    for(;;); // hang it for testing purposes
}

int set_idt_gate(int slot_number, void* handler_address) {
    uint64_t addr = (uint64_t)handler_address;
    idt[slot_number].offset_1 = addr & 0xFFFF;           // bits 0–15
    idt[slot_number].offset_2 = (addr >> 16) & 0xFFFF;   // bits 16–31
    idt[slot_number].offset_3 = (addr >> 32) & 0xFFFFFFFF; // bits 32–63
    idt[slot_number].selector = 0x0008;
    idt[slot_number].type_attributes = 0x8E;
    return 0;
}

int fill_idt_slots() {
    for(int i = 0; i < 256; i++) {
        set_idt_gate(i, unhandled_interrupt);
    }
    set_idt_gate(0, isr_divide_by_zero);
    set_idt_gate(14, isr_page_fault);
    set_idt_gate(32, irq0_handler);
    set_idt_gate(39, irq7_handler);
    set_idt_gate(47, irq15_handler);
    struct idtr idtr;
    // address of idtr array
    idtr.base  = (uint64_t)&idt;
    // size in bytes minus 1
    idtr.limit = sizeof(idt) - 1;
    __asm__ volatile ("lidt %0" : : "memory"(*&idtr));
    __asm__ volatile ("sti");
    return 0;
}
