#include "gdt.h"

uint64_t gdt[5];

int create_descriptor(int slot, uint32_t base, uint32_t limit, uint16_t flag)
{
    uint64_t descriptor;

    // Create the high 32 bit segment
    descriptor  =  limit       & 0x000F0000;         // set limit bits 19:16
    descriptor |= (flag <<  8) & 0x00F0FF00;         // set type, p, dpl, s, g, d/b, l and avl fields
    descriptor |= (base >> 16) & 0x000000FF;         // set base bits 23:16
    descriptor |=  base        & 0xFF000000;         // set base bits 31:24
 
    // Shift by 32 to allow for low part of segment
    descriptor <<= 32;
 
    // Create the low 32 bit segment
    descriptor |= base  << 16;                       // set base bits 15:0
    descriptor |= limit  & 0x0000FFFF;               // set limit bits 15:0

    // insert into our gdt struct
    gdt[slot] = descriptor;
    return 0;
}

int init_gdt() {
    // gdt null descriptor
    create_descriptor(0 ,0, 0x00000000, 0);
    // kernel mode code segment
    create_descriptor(1, 0, 0xFFFFF, GDT_CODE_PL0);
    // kernel mode data segment
    create_descriptor(2, 0, 0xFFFFF, GDT_DATA_PL0);
    // user mode code segment
    create_descriptor(3, 0, 0xFFFFF, GDT_DATA_PL3);
    // user mode data segment
    create_descriptor(4, 0, 0xFFFFF, GDT_CODE_PL3);

    struct gdtr gdtr;
    gdtr.base  = (uint64_t)&gdt; // address of gdt array
    gdtr.limit = sizeof(gdt) - 1; // size in bytes minus
    __asm__ volatile ("lgdt %0" : : "memory"(*&gdtr));

    __asm__ volatile (
    "push $0x08\n"          // push kernel code selector
    "lea 1f(%%rip), %%rax\n" // load address of label 1 into rax
    "push %%rax\n"           // push return address
    "lretq\n"               // far return — reloads cs
    "1:\n"
    : : : "rax"
    );

    __asm__ volatile (
    "mov $0x10, %%ax\n"   // kernel data selector
    "mov %%ax, %%ds\n"
    "mov %%ax, %%es\n"
    "mov %%ax, %%fs\n"
    "mov %%ax, %%gs\n"
    "mov %%ax, %%ss\n"
    : : : "ax"
    );
    return 0;
}
