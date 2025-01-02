section .text
extern kMain
global _start

_start:
	;We disable interrupts, we have no IDT installed
	cli
    
	; Place the vboot header into first argument for kMain
	mov rcx, rbx

	call kMain
	mov rax, 0x000000000000DEAD

	.idle:
		hlt
		jmp .idle