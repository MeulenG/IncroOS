#include "../cpu/trap.h"
#include "../libc/print.h"



void main_kernel(void)
{
   char *string = "Hello and Welcome my dude";
   int64_t value = 0x123456789ABCD;
   init_idt();

   printk("%s\n", string);
   printk("This value is equal to %x", value);
}