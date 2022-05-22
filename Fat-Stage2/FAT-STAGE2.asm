[BITS 16]
[ORG 0x7e00]

main_Stage2: jmp Stage2_Main
;*******************************************************
;	Preprocessor directives 16-BIT MODE
;*******************************************************
%include "asmlib16.inc"

;******************************************************
;	ENTRY POINT FOR STAGE 2
;******************************************************

Stage2_Main:
    ;-------------------------------;
	;   CPU FEATURES	            ;
	;-------------------------------;
    mov [bPhysicalDriveNum],dl
    mov eax,0x80000000
    cpuid
    cmp eax,0x80000001
    jb Deathmessage16bit

    mov eax,0x80000001
    cpuid
    test edx,(1<<29)
    jz Deathmessage16bit
    test edx,(1<<26)
    jz Deathmessage16bit

	;-------------------------------;
	;   Load Kernel             	;
	;-------------------------------;
LoadKernel:
    mov si,ReadPacket
    mov word[si],0x10
    mov word[si+2],100
    mov word[si+4],0
    mov word[si+6],0x1000
    mov dword[si+8],6
    mov dword[si+0xc],0
    mov dl,[bPhysicalDriveNum]
    mov ah,0x42
    int 0x13
    jc  ReadErrorStage216bit


Loaderuser:
    mov si,ReadPacket
    mov word[si],0x10
    mov word[si+2],10
    mov word[si+4],0
    mov word[si+6],0x2000
    mov dword[si+8],106
    mov dword[si+0xc],0
    mov dl,[bPhysicalDriveNum]
    mov ah,0x42
    int 0x13
    jc  ReadErrorStage216bit

	;-------------------------------;
	;   Retrieve Memory Map         ;
	;-------------------------------;
GetMemInfoStart:
    mov eax,0xe820
    mov edx,0x534d4150
    mov ecx,20
    mov dword[0x9000],0
    mov edi,0x9008
    xor ebx,ebx
    int 0x15
    jc Deathmessage16bit

GetMemInfo:
    add edi,20
    inc dword[0x9000] 
    test ebx,ebx
    jz GetMemDone

    mov eax,0xe820
    mov edx,0x534d4150
    mov ecx,20
    int 0x15
    jnc GetMemInfo

	;-------------------------------;
	;   A20   Gate              	;
	;-------------------------------;
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

	;-------------------------------;
	;   Video Mode              	;
	;-------------------------------;
SetVideoMode:
    mov ax,3
    int 0x10
    
    cli
    ;-------------------------------;
	;   GDT and IDT             	;
	;-------------------------------;
    lgdt [gdt_descriptor]
    lidt [Idt32Ptr]

    mov eax,cr0
    or eax,1
    mov cr0,eax

    jmp 8:Stage3

ReadErrorStage216bit:
Deathmessage16bit:
End:
    hlt
    jmp End

align 32
bits  32
;******************************************************
;	ENTRY POINT FOR STAGE 3
;******************************************************
;*******************************************************
;	Preprocessor directives 32-BIT MODE
;*******************************************************
%include "asmlib32.inc"

Stage3:
	;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    mov ax,0x10
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,0x7c00

    mov byte[0xb8000],'P'
    mov byte[0xb8001],0xa

    cld
    mov edi,0x70000
    xor eax,eax
    mov ecx,0x10000/4
    rep stosd
    
    mov dword[0x70000],0x71003
    mov dword[0x71000],10000011b

    mov eax, (0xffff800000000000>>39)
    and eax, 0x1ff
    mov dword[0x70000+eax*8], 0x72003
    mov dword[0x72000], 10000011b

    lgdt [Gdt64Ptr]

    mov eax,cr4
    or eax,(1<<5)
    mov cr4,eax

    mov eax,0x70000
    mov cr3,eax

    mov ecx,0xc0000080
    rdmsr
    or eax,(1<<8)
    wrmsr

    mov eax,cr0
    or eax,(1<<31)
    mov cr0,eax

	;---------------------------------------;
	;   Clear screen and print success	    ;
	;---------------------------------------;

;	call		ClrScr32
;	mov		    ebx, msgpmode
;	call		Puts32

	;---------------------------------------;
	;   Jump to 64-BIT Mode			        ;
	;---------------------------------------;
    
    jmp 8:Stage4

PEnd:
    hlt
    jmp PEnd


align 64
bits  64
;******************************************************
;	ENTRY POINT FOR STAGE 4
;******************************************************
Stage4:
	;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    mov rsp,0x7c00

    cld
    mov rdi,0x200000
    mov rsi,0x10000
    mov rcx,51200/8
    rep movsq

    mov byte[0xb8000],'L'
    mov byte[0xb8001],0x0

    mov rax, 0xffff800000200000
    jmp rax
    
LEnd:
    hlt
    jmp LEnd

;*******************************************************
;	Data Section
;*******************************************************
bPhysicalDriveNum			db		0
ErrorMsg                    db  "Hahahaha, Fuck you, but at Stage2"
msgA20Message               db  "Enabling A20 Gate", 0x0D, 0x0A, 0x00
msggdtMessage               db  "Installing gdt", 0x0D, 0x0A, 0x00
msgidtMessage               db  "Installing idt", 0x0D, 0x0A, 0x00
ReadPacket:                 times 16 db 0
LoadingMsg                  db 0x0D, 0x0A, "Stage 2 Sucessfully Loaded", 0x00
Msg                         db  "Preparing to load operating system...",13,10,0
msgpmode                    db  0x0A, 0x0A, 0x0A, "               <[ OMOS 32-bit 10 ]>"
                            db  0x0A, 0x0A,             "           Basic 32 bit graphics demo in Assembly Language", 0

gdt_start:

gdt_null:           ; The mandatory null descriptor
    dd 0x0          ; dd = define double word (4 bytes)
    dd 0x0

gdt_code:           ; Code segment descriptor
    dw 0xffff       ; Limit (bites 0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10011010b    ; 1st flags, type flags
    db 11001111b    ; 2nd flags, limit (bits 16-19)
    db 0x0          ; Base (bits 24-31)

gdt_data:
    dw 0xffff       ; Limit (bites 0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10010010b    ; 1st flags, type flags
    db 11001111b    ; 2nd flags, limit (bits 16-19)
    db 0x0

gdt_end:            ; necessary so assembler can calculate gdt size below

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size

    dd gdt_start                ; Start adress of GDT

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

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


Gdt64:
    dq 0
    dq 0x0020980000000000

Gdt64Len: equ $-Gdt64


Gdt64Ptr: dw Gdt64Len-1
          dd Gdt64