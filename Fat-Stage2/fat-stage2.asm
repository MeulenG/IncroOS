; 16 Bit Code, Origin at 0x7e00
bits	16							; We are still in 16 bit Real Mode

org		0x7e00						; We are loaded by BIOS at 0x7C00

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

Stage2_Main:
    mov ah,0x13
    mov al,1
    mov bx,0xa
    xor dx,dx
    mov bp,Message
    mov cx,MessageLen 
    int 0x10
    
End:
    hlt
    jmp End

Message:    db "loader starts"
MessageLen: equ $-Message