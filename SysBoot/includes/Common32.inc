%ifndef __COMMON32_INC
%define __COMMON32_INC
BITS 32
SetStack32:
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    mov     ax, 0x20
    mov     ds, ax
    mov     es, ax
    mov     ss, ax
    mov     esp, 0x7c00
    ret
%endif