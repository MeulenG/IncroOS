BITS    64
ALIGN   64


jmp Kernel_64_Main
;*******************************************************
;	Preprocessor directives
;*******************************************************
%include "../includes/Gdt.inc"
%include "../includes/GlobalDefines.inc"
%include "../includes/Idt.inc"
;******************************************************
;	ENTRY POINT For Kernel 64-Bit Mode
;******************************************************
Kernel_64_Main:
    xchg bx, bx
	;-------------------------------;
	;   Set registers		        ;
	;-------------------------------;
	mov	ax, 0x10		; set data segments to data selector (0x10)
	mov	ds, ax
	mov	ss, ax
	mov	es, ax
	mov	esp, 90000h		; stack begins from 90000h
Kernel_64_End:
    hlt
    
    jmp     Kernel_64_End