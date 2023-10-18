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
	mov rdi,Idt
    mov rax,Handler0

    mov [rdi],ax
    shr rax,16
    mov [rdi+6],ax
    shr rax,16
    mov [rdi+8],eax

    lgdt [TemporaryGdt]
    lidt [IdtPtr]

    push 8
    push KernelEntry
    db 0x48
    retf

KernelEntry:
    mov byte[0xb8000],'K'
    mov byte[0xb8001],0xa

    xor rbx,rbx
    div rbx

Handler0:
    mov byte[0xb8000],'D'
    mov byte[0xb8001],0xc

    jmp End

    iretq
	
Kernel_64_End:
    hlt
    
    jmp     Kernel_64_End