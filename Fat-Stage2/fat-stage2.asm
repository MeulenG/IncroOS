; 16 Bit Code, Origin at 0x7e00
bits	16							; We are still in 16 bit Real Mode

org		0x7e00						; We are loaded by BIOS at 0x7C00

;*******************************************************
;	Preprocessor directives
;*******************************************************

%include "stdio.inc"
%include "Gdt.inc"
%include "A20.inc"

;*******************************************************
;	Data Section
;*******************************************************
LoadingMsg db 0x0D, 0x0A, "Stage 2 Sucessfully Loaded", 0x00

;***************************************
;	Prints a string in 16-bit mode
;	DS=>SI: 0 terminated string
;***************************************

Print16bitmode:
			lodsb					; load next byte from string from SI to AL
			or			al, al		; Does AL=0?
			jz			PrintDone16bitmode	; Yep, null terminator found-bail out
			mov			ah,	0eh	; Nope-Print the character
			int			10h
			jmp			Print16bitmode		; Repeat until null terminator found
PrintDone16bitmode:
			ret					; we are done, so return

;*******************************************************
;	STAGE 2 ENTRY POINT
;
;		-Store BIOS information
;		-Setup the stack
;		-Load Kernel
;		-Install GDT
;		-Enable A20
;		-Jump to Stage 3
;*******************************************************

Stage2_Main:
	;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    cli

    xor ax, ax
    mov	ds, ax
    mov	es, ax
    mov	fs, ax
    mov	gs, ax

    mov	ss, ax
    mov	ax, 0x7C00
    mov	sp, ax

    sti
	;-------------------------------;
	;   Install our GDT		;
	;-------------------------------;

	call	InstallGDT		; install our GDT

	;-------------------------------;
	;   Enable A20			;
	;-------------------------------;

	call	EnableA20_KKbrd_Out ;Enable A20 Gate

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

	;-------------------------------;
	;   Go into pmode		;
	;-------------------------------;

    
End:
    hlt
    jmp End

;*******************************************************
;	Data Section
;*******************************************************
LoadingMsg db 0x0D, 0x0A, "Stage 2 Sucessfully Loaded", 0x00
MessageLen: equ $-LoadingMsg

msgpmode db  0x0A, 0x0A, 0x0A, "               <[ OS Development Series Tutorial 10 ]>"
    db  0x0A, 0x0A,             "           Basic 32 bit graphics demo in Assembly Language", 0