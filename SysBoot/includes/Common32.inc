%ifndef __COMMON32_INC
%define __COMMON32_INC
BITS 32
SetStack32:
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    mov     ax, DATA_SEGMENT
    mov     ds, ax
    mov     es, ax
    mov     ss, ax
    mov     esp, MEMLOCATION_INITIALLOWERSTACK
    jmp Continue_Part2
%endif