#ifndef __INTERRUPT_HANDLER_H__
#define __INTERRUPT_HANDLER_H__

#include <stdint.h>
#include "pic.h"
#include "../drivers/serial.h"
#include "../output/terminal.h"
#include "../cpu/io.h"
#include "../../libs/libkernel/print.h"
#include "../drivers/keyboard.h"

struct interrupt_frame {
    uintptr_t rip;
    uintptr_t cs;
    uintptr_t rflags;
    uintptr_t rsp;
    uintptr_t ss;
};


__attribute__((interrupt)) void isr_divide_by_zero(struct interrupt_frame* frame);
__attribute__((interrupt)) void isr_page_fault(struct interrupt_frame* frame, uintptr_t error_code);
__attribute__((interrupt)) void irq0_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq1_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq7_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq15_handler(struct interrupt_frame* frame);

#endif
