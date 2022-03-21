[BITS   16]
[ORG    0x7e00]

loader_start:
    mov [DRIVE], dl

    mov eax, 0x80000000 ; by passing this value we get CPU features
    cpuid ; Returns cpu information
    cmp eax, 0x80000001 ; We check for 64-bit support AKA long mode support, so if its below, then we jump to an error message
    jb NotSupported ; if this isnt the case, then we tell them the OS isnt supported
    mov eax, 0x80000001 ;
    cpuid
    test edx, (1<<29)
    jz NotSupported
    test edx, (1<<26) ; We check for 1gb paging, if it isnt inside, then we jump
    jz NotSupported

    mov ah,0x13
    mov al,1
    mov bx,0xa
    xor dx,dx
    mov bp,Message16bitmode
    mov cx,MessageLen16bitmode
    int 0x10

ReadKernel:
    mov si, ReadPacket
    mov word[si], 0x10
    mov word[si+2], 1
    mov word[si+4], 0x1000
    mov word[si+6], 0
    mov dword[si+8], 1
    mov dword[si+0xc], 0
    mov dl, [DRIVE]
    mov ah, 0x42
    int 0x13
    jc NotSupported



MemoryMap:
    mov  eax, 0xe820
    mov edx, 0x534d4150
    mov edx, 20
    mov dword[0x9000], 0
    mov edi, 0x9008
    xor ebx, ebx
    int 0x15
    jc NotSupported

GetMemoryInfo:
    add edi, 20
    inc dword[0x9000]
    test ebx, ebx
    jnz GetMemoryInfo
    
    mov  eax, 0xe820
    mov edx, 0x534d4150
    mov edx, 20
    int 0x15
    jnc GetMemoryInfo

GetMemoryMapSucess:
A20:
    mov ax,0xffff
    mov es, ax
    mov word[ds:0x7c00], 0xa200
    cmp word[es:0x7c10], 0xa200
    jne EnableA20
    mov word[0x7c00], 0xb200
    cmp word[es:0x7c10], 0xb200
    je loader_end

EnableA20:
    xor ax, ax
    mov es, ax


VideoMode:
    mov ax, 3 ;test mode
    int 0x10
    cli 
    lgdt [Gdt32Pointer]
    lidt [Idt32Pointer]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 8:PMEntry

NotSupported:
loader_end:
    hlt
    jmp loader_end


[BITS   32]
PMEntry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x7c00

    cld
    mov edi, 0x80000
    xor eax, eax
    mov ecx, 0x10000/4
    rep stosd

    mov dword[0x80000], 0x81007
    mov dword[0x81000], 10000111b
    
    lgdt [Gdt64Pointer]

    mov eax, cr4
    or eax, (1<<5)
    mov cr4, eax

    mov eax, 0x80000
    mov cr3, eax

    mov ecx, 0xc0000080
    rdmsr
    or eax, (1<<8)
    wrmsr

    mov eax, cr0
    or eax, (1<<31)
    mov cr0, eax

    jmp 8:LMEntry

PEnd:
    hlt
    jmp PEnd

[BITS   64]
LMEntry:
    mov rsp, 0x7c00
    cld
    mov rdi, 0x200000
    mov rsi, 0x10000
    mov rcx, 51200/8
    rep movsq ; since its 64-bit mode

    jmp 0x200000

LEnd:
    hlt
    jmp LEnd

DRIVE: db 0
ReadPacket: times 16 db 0

Gdt32:
    dq 0
Code32:
    dw 0xffff
    dw 0
    db 0
    db 0x9a
    db 0xcf
    db 0
Data32:
    dw 0xffff
    dw 0
    db 0
    db 0x92
    db 0xcf
    db 0
Gdt32Len: equ $-Gdt32
Gdt32Pointer: dw Gdt32Len-1
          dd Gdt32

Idt32Pointer:
dw 0x3ff
dd 0

Gdt64Len: equ $-Gdt64

Gdt64:
    dq 0
    dq 0x0020980000000000


Gdt64Pointer: dw Gdt64Len-1
          dd Gdt64



;*************************************************;
;	LONG        MODE        SUPPORT
;*************************************************;
Message16bitmode: db "Started in 16-bit Real Mode!"
MessageLen16bitmode: equ $-Message16bitmode



;*************************************************;
;	LONG        MODE        SUPPORT
;*************************************************;
MessageLongModeSupport: db "Long Mode Is Supported!"
MessageLenLongModeSupport: equ $-MessageLongModeSupport


;*************************************************;
;	KERNEL        LOAD          MESSAGE
;*************************************************;
MessageKernelLoad: db "Kernel Is Loaded"
MessageLenKernelLoad: equ $-MessageKernelLoad

;*************************************************;
;	MEMORY        MAP           MESSAGE
;*************************************************;
MessageMemory: db "Get Memory Info done"
MessageLenMemory: equ $-MessageMemory
;*************************************************;
;	A        20          MESSAGE
;*************************************************;
MessageA20: db "A20 Gate Is Enabled"
MessageLenA20: equ $-MessageA20
;*************************************************;
;	VIDEO        MODE          MESSAGE
;*************************************************;
MessageVideoMode: db "Video Mode Set, Success"
MessageLenVideoMode: equ $-MessageVideoMode