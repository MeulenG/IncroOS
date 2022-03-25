#include "trap.h"

static struct IdtPointer IdtPointer;
static struct IdtEntry InterruptVectors[256]; //256 items/interrupts
static struct TrapFrame TrapFrame;

static void init_idt_entry(struct IdtEntry *IdtEntry, uint64_t addr, uint8_t attribute)
{
    IdtEntry->low = (uint16_t)addr;
    IdtEntry->selector = 8;
    IdtEntry->attr = attribute;
    IdtEntry->mid = (uint16_t)(addr>>16);
    IdtEntry->high = (uint32_t)(addr>>32);
}

void init_idt(void)
{
    init_idt_entry(&InterruptVectors[0],(uint64_t)vector0,0x8e);
    init_idt_entry(&InterruptVectors[1],(uint64_t)vector1,0x8e);
    init_idt_entry(&InterruptVectors[2],(uint64_t)vector2,0x8e);
    init_idt_entry(&InterruptVectors[3],(uint64_t)vector3,0x8e);
    init_idt_entry(&InterruptVectors[4],(uint64_t)vector4,0x8e);
    init_idt_entry(&InterruptVectors[5],(uint64_t)vector5,0x8e);
    init_idt_entry(&InterruptVectors[6],(uint64_t)vector6,0x8e);
    init_idt_entry(&InterruptVectors[7],(uint64_t)vector7,0x8e);
    init_idt_entry(&InterruptVectors[8],(uint64_t)vector8,0x8e);
    init_idt_entry(&InterruptVectors[10],(uint64_t)vector10,0x8e);
    init_idt_entry(&InterruptVectors[11],(uint64_t)vector11,0x8e);
    init_idt_entry(&InterruptVectors[12],(uint64_t)vector12,0x8e);
    init_idt_entry(&InterruptVectors[13],(uint64_t)vector13,0x8e);
    init_idt_entry(&InterruptVectors[14],(uint64_t)vector14,0x8e);
    init_idt_entry(&InterruptVectors[16],(uint64_t)vector16,0x8e);
    init_idt_entry(&InterruptVectors[17],(uint64_t)vector17,0x8e);
    init_idt_entry(&InterruptVectors[18],(uint64_t)vector18,0x8e);
    init_idt_entry(&InterruptVectors[19],(uint64_t)vector19,0x8e);
    init_idt_entry(&InterruptVectors[32],(uint64_t)vector32,0x8e);
    init_idt_entry(&InterruptVectors[39],(uint64_t)vector39,0x8e);

    IdtPointer.limit = sizeof(InterruptVectors)-1;
    IdtPointer.addr = (uint64_t)InterruptVectors;
    load_idt(&IdtPointer);
}


void handler()
{
    unsigned char isr_value;

    switch (TrapFrame.trapno) {
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