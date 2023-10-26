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

KernelEntry:

Kernel_64_End:
    hlt
    
    jmp     Kernel_64_End