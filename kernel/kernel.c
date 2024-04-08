#include "stdarg.h"
#include "stddef.h"
#include "stdint.h"


void kernel_main(void) {
	// Write 'A' to the screen
	unsigned char * video = 0xB8000;
	*video = 'A';
	unsigned char *B = 0xB8002;
	*B = 'B';



	asm("hlt");
}