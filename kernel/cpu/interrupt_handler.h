#ifndef __INTERRUPT_HANDLER_H__
#define __INTERRUPT_HANDLER_H__

#include <stdint.h>

struct interrupt_frame {
    uintptr_t rip;
    uintptr_t cs;
    uintptr_t rflags;
    uintptr_t rsp;
    uintptr_t ss;
};


__attribute__((interrupt)) void isr_divide_by_zero(struct interrupt_frame* frame);
__attribute__((interrupt)) void isr_page_fault(struct interrupt_frame* frame, uintptr_t error_code);

#endif
