bits	16							; We are loaded in 16-bit Real Mode

org 0x7E00

jmp Stage2_Main
;*******************************************************
;	Preprocessor directives
;*******************************************************
%include "stdio.inc"		; basic i/o routines
;%include "Gdt.inc"			; Gdt routines
%include "A20.inc"			; A20 enabling


;******************************************************
;	ENTRY POINT FOR STAGE 2
;******************************************************
Stage2_Main:
	;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
	cli
    mov [DriveId], dl
    
    xor ax, ax
    mov	ds, ax
    mov	es, ax
    mov	fs, ax
    mov	gs, ax

    mov	ss, ax
    mov	ax, 0x7c00
    mov	sp, ax
    sti
    ;-------------------------------;
	;   Install our GDT         	;
	;-------------------------------;
	lgdt    [gdt_descriptor]	; Load our GDT
    mov si, msggdtMessage
    call    Puts16
    lidt    [Idt16]             ; Load our IDT
    mov si, msgidtMessage
    call    Puts16
    
    ;-------------------------------;
	;   Enable A20			        ;
    ;-------------------------------;
	call	A20MethodBios   ; Enable A20 gate through BIOS
    mov si, msgA20Message
    call    Puts16
    ;-------------------------------;
	;   Print loading message	    ;
	;-------------------------------;
    mov si, LoadingMsg
    call    Puts16

    ;-------------------------------;
	;   Go into pmode		        ;
	;-------------------------------;
EnterStage3:
	cli				    ; clear interrupts
	mov	eax, cr0		; set bit 0 in cr0--enter pmode
	or	eax, 1
	mov	cr0, eax


	jmp 0x8:Stage3	; far jump to fix CS

Stage2Error:
End:
    hlt
    jmp End

align 32
bits  32
;******************************************************
;	ENTRY POINT FOR STAGE 3
;******************************************************
Stage3:

	;-------------------------------;
	;   Set registers		;
	;-------------------------------;

    mov ax,0x10
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,0x7c00

	;---------------------------------------;
	;   Clear screen and print success	;
	;---------------------------------------;

	call		ClrScr32
	mov		ebx, msgpmode
	call		Puts32

	;---------------------------------------;
	;   Stop execution			;
	;---------------------------------------;

	cli
	hlt

;*******************************************************
;	Data Section
;*******************************************************
ErrorMsg        db  "gg no re"
msgA20Message   db  "Enabling A20 Gate", 0x0D, 0x0A, 0x00
msggdtMessage   db  "Installing gdt", 0x0D, 0x0A, 0x00
msgidtMessage   db  "Installing idt", 0x0D, 0x0A, 0x00
DriveId:        db 0
ReadPacket:     times 16 db 0
LoadingMsg      db 0x0D, 0x0A, "Stage 2 Sucessfully Loaded", 0x00
Msg             db  "Preparing to load operating system...",13,10,0
msgpmode        db  0x0A, 0x0A, 0x0A, "               <[ OS Development Series Tutorial 10 ]>"
                db  0x0A, 0x0A,             "           Basic 32 bit graphics demo in Assembly Language", 0
;*******************************************************
;	Preprocessor Critical Functions
;*******************************************************
;--------------------------------------
; Enables A20 line
;--------------------------------------
A20MethodBios:
	mov ax, 0x2401
	int 0x15
	ret
;--------------------------------------
; GDT   AND    IDT
;--------------------------------------
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

InstallGDT:
    lgdt [Gdt32Ptr]
    ret

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

; Interrupt Descriptor Table
Idt16:
    dw 0x3ff		; 256 entries, 4b each = 1K
    dd 0			; Real Mode IVT @ 0x0000
