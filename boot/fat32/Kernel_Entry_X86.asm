BITS 64
global _start

_start:
    mov rsp, 0x200000

hang:
    hlt