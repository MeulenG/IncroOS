#include "../cpu/trap.h"
#include "../libc/print.h"
#include "debug.h"
#include "../libc/memory.h"

void main_kernel(void)
{
   init_idt();
   init_memory();
   
}
