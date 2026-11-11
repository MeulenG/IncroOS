section .text
extern kMain
global _start

_start:
	;We disable interrupts, we have no IDT installed
	cli
    
	; Stage2 leaves the boot info block in RDI, which is already kMain's first
	; argument in the System V ABI gcc uses, so it goes straight through

	call kMain
	mov rax, 0x000000000000DEAD

	.idle:
		hlt
		jmp .idle