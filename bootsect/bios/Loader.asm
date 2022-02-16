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

 ********************************
; PrintNumber16
; @brief Prints out a digit to the screen at current position.
; 
; @param number_low  [In] The low word of the number to print
; @param number_high [In] The high word of the number to print
; @return            None
; ********************************
PrintNumber16:
    push bx

    ; load number into eax
    mov eax, dword

    xor bx, bx
    mov ecx, 10

    .convert:
        xor edx, edx
        div ecx

        ; now eax <-- eax/10
        ;     edx <-- eax % 10

        ; print edx
        ; this is one digit, which we have to convert to ASCII
        ; the print routine uses edx and eax, so let's push eax
        ; onto the stack. we clear edx at the beginning of the
        ; loop anyway, so we don't care if we much around with it

        ; convert dl to ascii
        add dx, 48
        push dx      ; store it for print
        inc  bx

        ; if eax is zero, we can quit
        cmp eax, 0
        jnz .convert

    .print:
        call PrintChar16
        pop ax

        dec bx
        jnz .print

    pop bx
    ret

NotSupported:
loader_end:
    hlt
    jmp loader_end

;*************************************************;
;	LONG        MODE        SUPPORT
;*************************************************;
DRIVE: db 0
MessageLongModeSupport: db "Long Mode Is Supported!"
MessageLen: equ $-MessageLongModeSupport
ReadPacket: times 16 db 0

;*************************************************;
;	KERNEL        LOAD          MESSAGE
;*************************************************;
MessageKernelLoad: db "Kernel Is Loaded"
MessageLen: equ $-MessageKernelLoad

;*************************************************;
;	MEMORY        MAP           MESSAGE
;*************************************************;
MessageMemory: db "Get Memory Info done"
MessageLen: equ $-MessageMemory
;*************************************************;
;	A        20          MESSAGE
;*************************************************;
MessageA20: db "A20 Gate Is Enabled"
MessageLen: equ $-MessageA20
;*************************************************;
;	VIDEO        MODE          MESSAGE
;*************************************************;
MessageVideoMode: db "Video Mode Set, Success"
MessageLen: equ $-MessageVideoMode
;*************************************************;
;	PROTECTED        MODE          MESSAGE
;*************************************************;
MessageProtectedMode: db "Protected Mode, Entered"
MessageLen: equ $-MessageProtectedMode
;*************************************************;
;	LONG        MODE          MESSAGE
;*************************************************;
MessageLongMode: db "Long Mode, Entered"
MessageLen: equ $-Message