[BITS   16]
[ORG    0x7e00]

loader_start:
    mov ah, 0x13 ; Function code
    mov al, 1 ; Cursor gets placed at the end of the string
    mov bx, 0xd ;0xa means it will be printed in green
    xor dx, dx ; Prints message at the beginning of the screen so we set it to 0
    mov bp, Message16bitmode ;Message displayed
    mov cx, MessageLen16bitmode ; Copies the characters to cx
    int 0x10 ;interrupt

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

    mov [DRIVE], dl

    mov eax, 0x80000000 ; by passing this value we get CPU features
    cpuid ; Returns cpu information
    cmp eax, 0x80000001 ; We check for 64-bit support AKA long mode support, so if its below, then we jump to an error message
    jb .NoLongMode ; if this isnt the case, then we tell them the OS isnt supported
    mov eax, 0x80000001 ;
    cpuid
    test edx, 1 << 29
    jz NotSupported
    test edx, 1 << 26 ; We check for 1gb paging, if it isnt inside, then we jump
    jz NotSupported


.NoLongMode:
    stc
    ret

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
    lidt [idtReal]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 8:PMEntry

NotSupported:
loader_end:
    hlt
    jmp loader_end

;*************************************************;
; Protected mode is the main operating mode of modern Intel processors (and clones) since the 80286 (16 bit). 
; On 80386s and later, the 32 bit Protected Mode allows working with several virtual address spaces, each of which has a maximum of 4GB of addressable memory;
; and enables the system to enforce strict memory and hardware I/O protection as well as restricting the available instruction set via Rings.
;*************************************************;
ALIGN  32
BITS   32
PMEntry:
    cli ; disable interrupts
    lgdt[Gdt64Pointer] ; load GDT register with start address of Global Descriptor Table
    mov eax, cr4
    or eax, 0x10 ; set PE (Protection Enable) bit in CR0 (Control Register 0)
    or eax, 0x20 ; 2mb pages
    mov cr4, eax

    ; Compatability mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100
    wrmsr
    ;Enable Paging
    mov eax, cr0
    or al, 1 ; set PE (Protection Enable) bit in CR0 (Control Register 0)
    mov cr0, eax
    ; Perform far jump to selector 08h (offset into GDT, pointing at a 32bit PM code segment descriptor) 
    ; to load CS with proper PM32 descriptor)
    jmp 08h:PModeMain

PModeMain:
    ; load DS, ES, FS, GS, SS, ESP
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov gs, ax
    mov esp, 0x7c00

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

idtReal:
	dw 0x3ff		; 256 entries, 4b each = 1K
	dd 0			; Real Mode IVT @ 0x0000

Gdt64Len: equ $-Gdt64

Gdt64:
    dq 0
    dq 0x0020980000000000


Gdt64Pointer: dw Gdt64Len-1
          dd Gdt64



;*************************************************;
;	16        BIT        MODE
;*************************************************;
Message16bitmode: db "Started in 16-bit Real Mode!"
MessageLen16bitmode: equ $-Message16bitmode