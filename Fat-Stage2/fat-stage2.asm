[BITS 16]
[ORG 0x7e00]

start:
    ; Check whether CPUID is supported or not
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 0x200000
    push eax
    popfd
    
    pushfd 
    pop eax
    xor eax, ecx
    shr eax, 21
    and eax, 1                        ; Check whether bit 21 is set or not. If EAX now contains 0, CPUID isn't supported.
    push ecx
    popfd
    
    test eax, eax
    jz .NoLongMode

    mov [DriveId], dl

    mov eax, 0x80000000 ; by passing this value we get CPU features
    cpuid ; Returns cpu information
    cmp eax, 0x80000001 ; We check for 64-bit support AKA long mode support, so if its below, then we jump to an error message
    jb .NoLongMode ; if this isnt the case, then we tell them the OS isnt supported
    mov eax, 0x80000001 ;
    cpuid
    test edx, 1 << 29
    jz .NoLongMode


.NoLongMode:
    stc
    ret

LoadKernel:
    mov si,ReadPacket
    mov word[si],0x10
    mov word[si+2],100
    mov word[si+4],0
    mov word[si+6],0x1000
    mov dword[si+8],6
    mov dword[si+0xc],0
    mov dl,[DriveId]
    mov ah,0x42
    int 0x13
    jc  ReadError

GetMemInfoStart:
    mov eax,0xe820
    mov edx,0x534d4150
    mov ecx,20
    mov edi,0x9000
    xor ebx,ebx
    int 0x15
    jc NotSupport

GetMemInfo:
    add edi,20
    mov eax,0xe820
    mov edx,0x534d4150
    mov ecx,20
    int 0x15
    jc GetMemDone

    test ebx,ebx
    jnz GetMemInfo

GetMemDone:
TestA20:
    mov ax,0xffff
    mov es,ax
    mov word[ds:0x7c00],0xa200
    cmp word[es:0x7c10],0xa200
    jne SetA20LineDone
    mov word[0x7c00],0xb200
    cmp word[es:0x7c10],0xb200
    je End
    
SetA20LineDone:
    xor ax,ax
    mov es,ax

SetVideoMode:
    mov ax,3
    int 0x10
    
    cli
    lgdt [Gdt32Ptr]
    lidt [Idt32Ptr]

    mov eax,cr0
    or eax,1
    mov cr0,eax

    jmp 8:PMEntry

ReadError:
NotSupport:
End:
    hlt
    jmp End

[BITS 32]
PMEntry:
    mov ax,0x10
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,0x7c00

    mov byte[0xb8000],'P'
    mov byte[0xb8001],0xa

    cld
    mov edi, 0x80000
    xor eax, eax
    mov ecx, 0x10000/4
    rep stosd

    mov dword[0x80000], 0x81007
    mov dword[0x81000], 10000111b

    jmp 8:LMEntry

PEnd:
    hlt
    jmp PEnd
    
ALIGN  64
BITS   64
LMEntry:
    cld
    mov rdi, 0x200000
    mov rsi, 0x10000
    mov rcx, 51200/8
    rep movsq

    mov byte[0xb8002],'L'
    mov byte[0xb8003],0xa
    
    ; clear out upper 32 bits of stack to make sure
    ; no intended bits are left up there
    mov eax, esp
    xor rsp, rsp
    mov rsp, rax
    
    ; clear out 64 bit parts of registers as we do not
    ; know the state of them, and we need to use them
    ; for passing state
    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    
    ;jump to the kernels address and never return
    jmp 0x200000

LEnd:
    hlt
    jmp LEnd

DriveId:    db 0
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

Gdt32Ptr: dw Gdt32Len-1
          dd Gdt32

Idt32Ptr: dw 0
          dd 0
