%ifndef __COMMON16_INC
%define __COMMON16_INC
BITS 16
SetStack16:
	; Clear Interrupts
    cli
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    xor     ax, ax
    mov	    ds, ax
    mov	    es, ax
    mov	    fs, ax
    mov	    gs, ax
    mov	    ss, ax
    mov	    ax, 0x7C00
    mov	    sp, ax
    ; Enable Interrupts
    sti
    ret
%endif