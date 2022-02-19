;*************************************************;
;	MEMORY MAP



;*************************************************;

[BITS   16]
[ORG    0x7e00]

loader_start:
    mov [DRIVE], dl

    mov eax, 0x80000000 ; by passing this value we get CPU features
    cpuid ; Returns cpu information
    cmp eax, 0x80000001 ; We check for 64-bit support AKA long mode support, so if its below, then we jump to an error message
    jb NotSupported ; 
    mov eax, 0x80000001 ;
    cpuid
    test edx, (1<<29) ;
    jz NotSupported
    test edx, (1<<26) ; We check for 1gb paging, if it isnt inside, then we jump
    jz NotSupported

    mov ah,0x13
    mov al,1
    mov bx,0xa
    xor dx,dx
    mov bp,MessageLongModeSupport
    mov cx,MessageLenLongModeSupport
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
    mov ah,0x13
    mov al,1
    mov bx,0xa
    xor dx,dx
    mov bp,MessageKernelLoad
    mov cx,MessageLenKernelLoad
    int 0x10

MemoryMap:
    mov  eax, 0xe820
    mov edx, 0x534d4150
    mov edx, 20
    mov edi, 0x9000
    xor ebx, ebx
    int 0x15
    jc NotSupported

GetMemoryInfo:
    add edi, 20
    mov  eax, 0xe820
    mov edx, 0x534d4150
    mov edx, 20
    int 0x15
    jc NotSupported

    test ebx, ebx
    jnz GetMemoryInfo

GetMemoryMapSucess:
    mov ah, 0x13 ; Function code
    mov al, 1 ; Cursor gets placed at the end of the string
    mov bx, 0xa ;0xa means it will be printed in green
    xor dx, dx ; Prints message at the beginning of the screen so we set it to 0
    mov bp, MessageMemory ;Message displayed
    mov cx, MessageLenMemory ; Copies the characters to cx
    int 0x10 ;interrupt

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
    mov ah, 0x13 ; Function code
    mov al, 1 ; Cursor gets placed at the end of the string
    mov bx, 0xa ;0xa means it will be printed in green
    xor dx, dx ; Prints message at the beginning of the screen so we set it to 0
    mov bp, MessageA20 ;Message displayed
    mov cx, MessageLenA20 ; Copies the characters to cx
    int 0x10 ;interrupt

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
;    mov si, MessageVideoMode
;    mov ax, 0xb800
;    mov es, ax
;    xor di, di
;    mov cx, MessageLenVideoMode
;Print:
;    mov al, [si]
;    mov [es:di], al
;    mov byte[es:di+1], 0xa
;
 ;   add di, 2
 ;   add si, 1
 ;   loop Print

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

    mov byte[0xb8000], 'L'
    mov byte[0xb8001], 0xa

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
Gdt32Len: equ $-Gdt32Pointer
Gdt32Pointer: dw Gdt32Len-1
        dd Gdt32

Idt32Pointer: dw 0
            dd 0

Gdt64Len: equ $-Gdt64Pointer

Gdt64:
    dq 0
    dq 0x002098000000000


Gdt64Pointer: dw Gdt64Len-1
                dd Gdt64
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
;*************************************************;
;	PROTECTED        MODE          MESSAGE
;*************************************************;
MessageProtectedMode: db "Protected Mode, Entered"
MessageLenProtectedMode: equ $-MessageProtectedMode
;*************************************************;
;	LONG        MODE          MESSAGE
;*************************************************;
MessageLongMode: db "Long Mode, Entered"
MessageLenLongMode: equ $-MessageLongMode