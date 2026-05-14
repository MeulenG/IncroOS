#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdint.h>

struct InterruptDescriptor64 {
   uint16_t offset_1;        // offset bits 0..15
   uint16_t selector;        // a code segment selector in GDT or LDT
   uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
   uint8_t  type_attributes; // gate type, dpl, and p fields
   uint16_t offset_2;        // offset bits 16..31
   uint32_t offset_3;        // offset bits 32..63
   uint32_t zero;            // reserved
};

struct idt_entry_64 {
    uint16_t offset_low;       // bits 0-15 of handler address
    uint16_t selector;         // GDT code segment selector
    uint8_t  ist;              // bits 0-2 = IST index; bits 3-7 = 0
    uint8_t  type_attr;        // gate type | DPL | P  (e.g., 0x8E = present, ring 0, interrupt gate)
    uint16_t offset_mid;       // bits 16-31 of handler address
    uint32_t offset_high;      // bits 32-63 of handler address
    uint32_t zero;             // reserved, must be 0
} __attribute__((packed));

struct idtr {
    uint16_t limit;     // size of IDT in bytes minus 1
    uint64_t base;      // address of IDT
} __attribute__((packed));

#endif // __SERIAL_H__
