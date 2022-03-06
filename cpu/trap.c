#include "trap.h"

static struct IdtPtr idt_pointer;
static struct IdtEntry vectors[256]; //256 items/interrupts

static void init_idt_entry(struct IdtEntry *entry, uint64_t addr, uint8_t attribute)
{
    entry->low = (uint16_t)addr;
    entry->selector = 8;
    entry->attr = attribute;
    entry->mid = (uint16_t)(addr>>16);
    entry->high = (uint32_t)(addr>>32);
}

void init_idt(void)
{
    init_idt_entry(&vectors[0],(intptr_t)vector0,0x8e);
    init_idt_entry(&vectors[1],(intptr_t)vector1,0x8e);
    init_idt_entry(&vectors[2],(intptr_t)vector2,0x8e);
    init_idt_entry(&vectors[3],(intptr_t)vector3,0x8e);
    init_idt_entry(&vectors[4],(intptr_t)vector4,0x8e);
    init_idt_entry(&vectors[5],(intptr_t)vector5,0x8e);
    init_idt_entry(&vectors[6],(intptr_t)vector6,0x8e);
    init_idt_entry(&vectors[7],(intptr_t)vector7,0x8e);
    init_idt_entry(&vectors[8],(intptr_t)vector8,0x8e);
    init_idt_entry(&vectors[10],(intptr_t)vector10,0x8e);
    init_idt_entry(&vectors[11],(intptr_t)vector11,0x8e);
    init_idt_entry(&vectors[12],(intptr_t)vector12,0x8e);
    init_idt_entry(&vectors[13],(intptr_t)vector13,0x8e);
    init_idt_entry(&vectors[14],(intptr_t)vector14,0x8e);
    init_idt_entry(&vectors[16],(intptr_t)vector16,0x8e);
    init_idt_entry(&vectors[17],(intptr_t)vector17,0x8e);
    init_idt_entry(&vectors[18],(intptr_t)vector18,0x8e);
    init_idt_entry(&vectors[19],(intptr_t)vector19,0x8e);
    init_idt_entry(&vectors[32],(intptr_t)vector32,0x8e);
    init_idt_entry(&vectors[39],(intptr_t)vector39,0x8e);

    idt_pointer.limit = sizeof(vectors)-1;
    idt_pointer.addr = (intptr_t)vectors;
    load_idt(&idt_pointer);
}

void handler(struct TrapFrame *tf)
{
    unsigned char isr_value;

    switch (tf->trapno) {
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