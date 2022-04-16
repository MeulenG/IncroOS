bits	16							; We are loaded in 16-bit Real Mode

org		0x7e00						; We are loaded by BIOS at 0x7C00

;******************************************************
;	ENTRY POINT FOR STAGE 2
;******************************************************
Stage2_Main:
	;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
	cli
    mov [DriveId],dl

    mov eax,0x80000000
    cpuid
    cmp eax,0x80000001
    jb Stage2Error
    
	sti
    ;-------------------------------;
	;   Install our GDT		;
	;-------------------------------;

	lgdt [InstallGDT]		; install our GDT

	;-------------------------------;
	;   Enable A20			;
	;-------------------------------;

	call	A20MethodBios
	
    ;-------------------------------;
	;   Print loading message	;
	;-------------------------------;
    mov ah,0x13
    mov al,1
    mov bx,0xa
    xor dx,dx
    mov bp,LoadingMsg
    mov cx,MessageLen 
    int 0x10

Stage2Error:
End:
    hlt
    jmp End

EnterStage3:

	cli				; clear interrupts
	mov	eax, cr0		; set bit 0 in cr0--enter pmode
	or	eax, 1
	mov	cr0, eax

	mov ah,0x13
    mov al,1
    mov bx,0xa
    xor dx,dx
    mov bp,TestMsg
    mov cx,MessageLenTest 
    int 0x10

	jmp	08h:Stage3	; far jump to fix CS

	; Note: Do NOT re-enable interrupts! Doing so will triple fault!
	; We will fix this in Stage 3.


align 32
bits  32
;*******************************************************
;	Preprocessor directives
;*******************************************************
%include "stdio.inc"
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

	CALL		ClrScr32
	mov		ebx, msgpmode
	CALL		Puts32

	;---------------------------------------;
	;   Stop execution			;
	;---------------------------------------;

	cli
	hlt

;*******************************************************
;	Data Section
;*******************************************************
ErrorMsg db  "gg no re"
MessageLenStage2Error: equ $-ErrorMsg
DriveId:    db 0
LoadingMsg db 0x0D, 0x0A, "Stage 2 Sucessfully Loaded", 0x00
MessageLen: equ $-LoadingMsg
Msg db  "Preparing to load operating system...",13,10,0
MessageLenOS: equ $-Msg
msgpmode db  0x0A, 0x0A, 0x0A, "               <[ OS Development Series Tutorial 10 ]>"
    db  0x0A, 0x0A,             "           Basic 32 bit graphics demo in Assembly Language", 0
MessageLenpmode: equ $-msgpmode
TestMsg:    db "Here"
MessageLenTest: equ $-TestMsg

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

;*******************************************
; InstallGDT()
;	- Install our GDT
;*******************************************
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
    
Gdt32Len: equ $-InstallGDT

InstallGDT: dw Gdt32Len-1
          	dd Gdt32