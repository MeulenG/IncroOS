section .text
extern kernel_main
global _start

_start:    
InitPIT:
    mov al, (1<<2)|(3<<4)
    out 0x43, al

    mov ax, 11931
    out 0x40, al
    mov al, ah
    out 0x40, al

InitPIC:
    mov al, 0x11
    out 0x20, al
    out 0xa0, al

    mov al, 32
    out 0x21, al
    mov al, 40
    out 0xa1, al

    mov al, 4
    out 0x21, al
    mov al, 2
    out 0xa1, al

    mov al, 1
    out 0x21, al
    out 0xa1, al

    mov al, 11111110b
    out 0x21, al
    mov al, 11111111b
    out 0xa1, al

    push 8
    push KernelEntry
    db 0x48
    retf
KernelEntry:
    mov rsp, 0x200000
    call kernel_main
hang:
    hlt
    jmp hang