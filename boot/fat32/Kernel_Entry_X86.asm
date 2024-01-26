.section .bss
.align 16
stack_bottom:
.skip 4096
stack_top:

.section .text
.global _start
_start:
    mov $stack_top, %rsp

    call kernel_main

    hlt

_end:
    hlt
    jmp  _end