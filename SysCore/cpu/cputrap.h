#ifndef _TRAP_H_
#define _TRAP_H_

#include "stdint.h"

struct InterruptDescriptor64 {
   uint16_t offset_1;        // offset bits 0..15
   uint16_t selector;        // a code segment selector in GDT or LDT
   uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
   uint8_t  type_attributes; // gate type, dpl, and p fields
   uint16_t offset_2;        // offset bits 16..31
   uint32_t offset_3;        // offset bits 32..63
   uint32_t zero;            // reserved
};

struct idt_Pointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct interrupt_Frame {
    int64_t r15;
    int64_t r14;
    int64_t r13;
    int64_t r12;
    int64_t r11;
    int64_t r10;
    int64_t r9;
    int64_t r8;
    int64_t rbp;
    int64_t rdi;
    int64_t rsi;
    int64_t rdx;
    int64_t rcx;
    int64_t rbx;
    int64_t rax;
    int64_t trapno;
    int64_t errorcode;
    int64_t rip;
    int64_t cs;
    int64_t rflags;
    int64_t rsp;
    int64_t ss;
};


void INTERRUPT0(void);
void INTERRUPT1(void);
void INTERRUPT2(void);
void INTERRUPT3(void);
void INTERRUPT4(void);
void INTERRUPT5(void);
void INTERRUPT6(void);
void INTERRUPT7(void);
void INTERRUPT8(void);
void INTERRUPT10(void);
void INTERRUPT11(void);
void INTERRUPT12(void);
void INTERRUPT13(void);
void INTERRUPT14(void);
void INTERRUPT16(void);
void INTERRUPT17(void);
void INTERRUPT18(void);
void INTERRUPT19(void);
void INTERRUPT32(void);
void INTERRUPT33(void);
void INTERRUPT39(void);
void sysint(void);
void init_idt(void);
void ENDOFINT(void);
void idt_Install(struct idt_Pointer *ptr);
unsigned char READ_ISR(void);
uint64_t read_cr2(void);
void TrapReturn(void);
uint64_t get_ticks(void);


#endif