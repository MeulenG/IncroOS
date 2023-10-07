%ifndef __COMMON64_INC
%define __COMMON64_INC
BITS 64
SetStack64:
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    xor rax, rax
    mov ax, 0x30
    mov ds, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov es, ax
    ret
%endif