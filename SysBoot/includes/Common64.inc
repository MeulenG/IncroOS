%ifndef __COMMON64_INC
%define __COMMON64_INC
BITS 64
SetStack64:
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;

    ; Clear general purpose registers
    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    xor rsi, rsi
    xor rdi, rdi
    xor rbp, rbp

    ; Set data segments
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Set the stack
    mov rsp, 0x7C00

    jmp Continue_Part3
%endif
