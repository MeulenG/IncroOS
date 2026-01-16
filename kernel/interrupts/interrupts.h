#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

// IDT entry structure for 64-bit mode
struct idt_entry {
    uint16_t offset_low;    // Offset bits 0-15
    uint16_t selector;      // Code segment selector in GDT
    uint8_t  ist;           // Interrupt Stack Table offset (bits 0-2), rest reserved
    uint8_t  type_attr;     // Type and attributes
    uint16_t offset_mid;    // Offset bits 16-31
    uint32_t offset_high;   // Offset bits 32-63
    uint32_t reserved;      // Reserved, must be zero
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
    uint16_t limit;         // Size of IDT - 1
    uint64_t base;          // Base address of IDT
} __attribute__((packed));

// Register state pushed by interrupt handlers
struct interrupt_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi;
    uint64_t int_no;        // Interrupt number
    uint64_t error_code;    // Error code (if applicable)
    uint64_t rip;           // Instruction pointer
    uint64_t cs;            // Code segment
    uint64_t rflags;        // CPU flags
    uint64_t userrsp;       // User stack pointer
    uint64_t ss;            // Stack segment
} __attribute__((packed));

// Function prototypes
void idt_init(void);
void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t flags);
void interrupt_handler(struct interrupt_frame* frame);

// External ISR declarations (defined in isr.asm)
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

// IRQ handlers (32-47)
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

// PIC functions
void pic_init(void);
void pic_send_eoi(uint8_t irq);

// PIT functions
void pit_init(uint32_t frequency);

#endif // INTERRUPTS_H
