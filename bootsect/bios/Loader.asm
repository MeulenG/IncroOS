;*************************************************;
;	MEMORY MAP



;*************************************************;



[BITS   16]
[ORG    0x7e00]

loader_start:
    mov [DRIVE], dl

    mov eax, 0x80000000 ; by passing this value we get processor features
    cpuid ; Returns cpu information
    cmp eax, 0x80000001 ; We check for 64-bit support AKA long mode support, so if its below, then we jump
    jb NotSupported
    mov eax, 0x80000001
    cpuid
    test edx, (1<<29) ;
    jz NotSupported
    test edx, (1<<26) ; We check for 1gb paging, if it isnt inside, then we jump
    jz NotSupported

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
    mov bp, Message ;Message displayed
    mov cx, MessageLen ; Copies the characters to cx
    int 0x10 ;interrupt

NotSupported:
loader_end:
    hlt
    jmp loader_end

;*************************************************;
;	LONG        MODE        SUPPORT
;*************************************************;
DRIVE: db 0
Message: db "Get Memory Info done"
MessageLen: equ $-Message
ReadPacket: times 16 db 0

;*************************************************;
;	KERNEL        LOAD          MESSAGE
;*************************************************;
Message: db "Get Memory Info done"
MessageLen: equ $-Message

;*************************************************;
;	MEMORY        MAP           MESSAGE
;*************************************************;
Message: db "Get Memory Info done"
MessageLen: equ $-Message
;*************************************************;
;	A        20          MESSAGE
;*************************************************;
Message: db "Get Memory Info done"
MessageLen: equ $-Message
;*************************************************;
;	VIDEO        MODE          MESSAGE
;*************************************************;
Message: db "Get Memory Info done"
MessageLen: equ $-Message
;*************************************************;
;	PROTECTED        MODE          MESSAGE
;*************************************************;
Message: db "Get Memory Info done"
MessageLen: equ $-Message
;*************************************************;
;	LONG        MODE          MESSAGE
;*************************************************;
Message: db "Get Memory Info done"
MessageLen: equ $-Message