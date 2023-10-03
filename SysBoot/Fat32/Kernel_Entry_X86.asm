BITS    64
ALIGN   64


jmp Kernel_64_Main
;*******************************************************
;	Preprocessor directives 64-BIT MODE
;*******************************************************
%include "../Routines/stdio64.inc"
%include "../Routines/Gdt.inc"
;******************************************************
;	ENTRY POINT For Kernel 64-Bit Mode
;******************************************************
Kernel_64_Main:
    xchg bx, bx
    lgdt [gdt_descriptor_64]

    push 8
    push KernelEntry
    db 0x48
    retf
    
KernelEntry:
    mov byte[0xb8000], 'K'
    mov byte[0xb8001], 0xa

Kernel_64_End:
    HLT
    
    JMP     Kernel_64_End

;*******************************************************
;	Data Section
;*******************************************************
KRNLDR_HandOver				DB 	0x0A, 0x0D "Control Handed To Kernel, Sucessfully", 0x00

Idt:
    %rep 256
        dw 0
        dw 0x8
        db 0
        db 0x8e
        dw 0
        dd 0
        dd 0
    %endrep

IdtLen: equ $-Idt

IdtPtr: dw IdtLen
        dq Idt

;*******************************************************
;	Data Error Section
;*******************************************************
KRNLDR_Error                DB  0x0A, 0x0D "Control Handed To Kernel, Unsucessfully", 0x00