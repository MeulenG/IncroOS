#include "cputrap.h"
#include "../Libraries/printLib.h"
#include "../Core/syscall.h"
#include "../Core/kerneldebugger.h"
#include "../Core/process.h"
#include "../drivers/Keyboard.h"

static struct idt_Pointer idt_Pointer;
static struct InterruptDescriptor64 interruptDescriptor64_Vector[256];
static uint64_t ticks;

static void init_idt_entry(struct InterruptDescriptor64 *InterruptDescriptor64_Entry, uint64_t base, uint8_t attribute)
{
    InterruptDescriptor64_Entry->offset_1 = (uint16_t)base;
    InterruptDescriptor64_Entry->selector = 8;
    InterruptDescriptor64_Entry->type_attributes = attribute;
    InterruptDescriptor64_Entry->offset_2 = (uint16_t)(base>>16);
    InterruptDescriptor64_Entry->offset_3 = (uint32_t)(base>>32);
}

void init_idt(void)
{
    init_idt_entry(&interruptDescriptor64_Vector[0],(uint64_t)INTERRUPT0,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[1],(uint64_t)INTERRUPT1,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[2],(uint64_t)INTERRUPT2,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[3],(uint64_t)INTERRUPT3,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[4],(uint64_t)INTERRUPT4,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[5],(uint64_t)INTERRUPT5,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[6],(uint64_t)INTERRUPT6,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[7],(uint64_t)INTERRUPT7,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[8],(uint64_t)INTERRUPT8,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[10],(uint64_t)INTERRUPT10,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[11],(uint64_t)INTERRUPT11,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[12],(uint64_t)INTERRUPT12,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[13],(uint64_t)INTERRUPT13,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[14],(uint64_t)INTERRUPT14,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[16],(uint64_t)INTERRUPT16,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[17],(uint64_t)INTERRUPT17,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[18],(uint64_t)INTERRUPT18,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[19],(uint64_t)INTERRUPT19,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[32],(uint64_t)INTERRUPT32,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[33],(uint64_t)INTERRUPT33,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[39],(uint64_t)INTERRUPT39,0x8E);
    init_idt_entry(&interruptDescriptor64_Vector[0x80],(uint64_t)sysint,0xee);

    idt_Pointer.limit = sizeof(interruptDescriptor64_Vector)-1;
    idt_Pointer.base = (uint64_t)interruptDescriptor64_Vector;
    idt_Install(&idt_Pointer);
}

uint64_t get_ticks(void)
{
    return ticks;
}

static void timer_handler(void)
{
    ticks++;
    wake_up(-1);
}

void isr_handler(struct interrupt_Frame*frame)
{
    unsigned char isr_value;

    switch (frame->trapno) {
        case 32:  
            timer_handler();   
            ENDOFINT();
            break;

        case 33:  
            keyboard_handler();
            ENDOFINT();
            break;
            
        case 39:
            isr_value = READ_ISR();
            if ((isr_value&(1<<7)) != 0) {
                ENDOFINT();
            }
            break;

        case 0x80:                   
            system_call(frame);
            break;

        default:
            if ((frame->cs & 3) == 3) {
            //    printk("Exception is %d\n", frame->trapno);
                exit();
            }
            else {
                while (1) { }
            }
    }

    if (frame->trapno == 32) {       
        yield();
    }
}