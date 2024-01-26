%ifndef __COMMON16_INC
%define __COMMON16_INC
BITS 16
SetStack16:
	;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
	cli				    ; clear interrupts
	xor	ax, ax			; null segments
	mov	ds, ax
	mov	es, ax
	mov	ax, 0x0			; stack begins at 0x00000500 - 0x00007BFF
	mov	ss, ax
	mov	sp, MEMLOCATION_INITIALUPPERSTACK
	sti				    ; enable interrupts
    jmp Continue_Part1
%endif