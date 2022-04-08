#include "trap.h"

static struct InterruptDescriptorTable64 g_idtEntries[256] = { { 0 } };
static struct IdtPointer g_idtPointer = { 0 };
static struct TrapFrame trapFrame;

static void init_idt_entry(struct InterruptDescriptorTable64 *IdtEntry, uint64_t offset, uint8_t type_attributes)
{
    IdtEntry->offset_1 = (uint16_t)offset;
    IdtEntry->selector = 8;
    IdtEntry->type_attributes = type_attributes;
    IdtEntry->offset_2 = (uint16_t)(offset>>16);
    IdtEntry->offset_3 = (uint32_t)(offset>>32);
}

void init_idt(void)
{
    init_idt_entry(&g_idtEntries[0],(uint64_t)vector0,0x8e);
    init_idt_entry(&g_idtEntries[1],(uint64_t)vector1,0x8e);
    init_idt_entry(&g_idtEntries[2],(uint64_t)vector2,0x8e);
    init_idt_entry(&g_idtEntries[3],(uint64_t)vector3,0x8e);
    init_idt_entry(&g_idtEntries[4],(uint64_t)vector4,0x8e);
    init_idt_entry(&g_idtEntries[5],(uint64_t)vector5,0x8e);
    init_idt_entry(&g_idtEntries[6],(uint64_t)vector6,0x8e);
    init_idt_entry(&g_idtEntries[7],(uint64_t)vector7,0x8e);
    init_idt_entry(&g_idtEntries[8],(uint64_t)vector8,0x8e);
    init_idt_entry(&g_idtEntries[10],(uint64_t)vector10,0x8e);
    init_idt_entry(&g_idtEntries[11],(uint64_t)vector11,0x8e);
    init_idt_entry(&g_idtEntries[12],(uint64_t)vector12,0x8e);
    init_idt_entry(&g_idtEntries[13],(uint64_t)vector13,0x8e);
    init_idt_entry(&g_idtEntries[14],(uint64_t)vector14,0x8e);
    init_idt_entry(&g_idtEntries[16],(uint64_t)vector16,0x8e);
    init_idt_entry(&g_idtEntries[17],(uint64_t)vector17,0x8e);
    init_idt_entry(&g_idtEntries[18],(uint64_t)vector18,0x8e);
    init_idt_entry(&g_idtEntries[19],(uint64_t)vector19,0x8e);
    init_idt_entry(&g_idtEntries[32],(uint64_t)vector32,0x8e);
    init_idt_entry(&g_idtEntries[39],(uint64_t)vector39,0x8e);

    g_idtPointer.limit = sizeof(g_idtEntries)-1;
    g_idtPointer.base = (uint64_t)&g_idtEntries[0];
    load_idt(&g_idtPointer);
}


void handler()
{
    unsigned char isr_value;

    switch (trapFrame.trapno) {
        case 32:
            eoi();
            break;
            
        case 39:
            isr_value = read_isr();
            if ((isr_value&(1<<7)) != 0) {
                eoi();
            }
            break;

        default:
            while (1) { }
    }
}