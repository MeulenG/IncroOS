#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>
#include "../drivers/serial.h"
#include "../output/terminal.h"

struct InterruptDescriptor64 {
   uint16_t offset_1;        // offset bits 0..15
   uint16_t selector;        // a code segment selector in GDT or LDT
   uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
   uint8_t  type_attributes; // gate type, dpl, and p fields
   uint16_t offset_2;        // offset bits 16..31
   uint32_t offset_3;        // offset bits 32..63
   uint32_t zero;            // reserved
};

// IDT is a a lookup table with 256 slots(one per interrupt number) 
extern struct InterruptDescriptor64 idt[256];

struct idtr {
    uint16_t limit;     // size of IDT in bytes minus 1
    uint64_t base;      // address of IDT
} __attribute__((packed));

void unhandled_interrupt();
int set_idt_gate(int slot_number, void* handler_address);
int fill_idt_slots();

#endif // __SERIAL_H__
