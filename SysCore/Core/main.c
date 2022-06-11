#include "../cpu/cputrap.h"
#include "../../SysLib/libc/Print/printLib.h"
#include "../../SysLib/libc/Memory/memLib.h"
#include "../PManagement/process.h"
#include "../../SysCalls/syscall.h"

void KMain(void)
{
   init_idt();
   init_memory();  
   init_kvm();
   init_system_call();
   init_process();
   launch();
}