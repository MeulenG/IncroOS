#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "../SysCore/cpu/cputrap.h"

typedef int (*SYSTEMCALL)(int64_t *argptr);
void init_system_call(void);
void system_call(struct interrupt_Frame *frame);

#endif