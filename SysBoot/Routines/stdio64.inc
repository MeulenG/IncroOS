%ifndef __ASMLIB64_INC
%define __ASMLIB64_INC

bits 64
%macro SAVE_STATE 0
    PUSH    r15
    
    PUSH    r14
    
    PUSH    r13
    
    PUSH    r12
    
    PUSH    r11
    
    PUSH    r10
    
    PUSH    r9
    
    PUSH    r8

    PUSH    rax
    
    PUSH    rcx
    
    PUSH    rdx
    
    PUSH    rbx
    
    PUSH    rsp
    
    PUSH    rbp
    
    PUSH    rsi
    
    PUSH    rdi
%endmacro

%macro RESTORE_STATE 0
    ; Restore registers
    POP     rdi
    
    POP     rsi
    
    POP     rbp
    
    ADD     rsp, 8 ; skip rsp
    
    POP     rbx
    
    POP     rdx
    
    POP     rcx
    
    POP     rax

    POP     r8
    
    POP     r9
    
    POP     r10
    
    POP     r11
    
    POP     r12
    
    POP     r13
    
    POP     r14
    
    POP     r15

	; Cleanup irq & error code from stack
    ADD     rsp, 0x10
%endmacro


%define		VIDMEM	0xB8000			; video memory
%define		COLS	80				; width and height of screen
%define		LINES	25
%define		CHAR_ATTRIB 63			; character attribute (White text on a turquois background)

_CurX64 db 0					; current x/y location
_CurY64 db 0

;**************************************************;
;	Putch64 ()
;		- Prints a character to screen
;	BL => Character to print
;**************************************************;

Putch64:
	SAVE_STATE				; macro for pusha
	MOV	    rdi, VIDMEM		; get pointer to video memory

	;-------------------------------;
	;   Get current position	    ;
	;-------------------------------;

	XOR	    rax, rax	    ; clear eax

		;--------------------------------
		; Remember: currentPos = x + y * COLS! x and y are in _CurX64 and _CurY64.
		; Because there are two bytes per character, COLS=number of characters in a line.
		; We have to multiply this by 2 to get number of bytes per line. This is the screen width,
		; so multiply screen with * _CurY64 to get current line
		;--------------------------------

		MOV	    rcx, COLS*2		    ; Mode 7 has 2 bytes per char, so its COLS*2 bytes per line
		
        MOV	    al, byte [_CurY64]	; get y pos
		
        MUL	    rcx			        ; multiply y*COLS
		
        PUSH    rax			        ; save eax--the multiplication

		;--------------------------------
		; Now y * screen width is in eax. Now, just add _CurX64. But, again remember that _CurX64 is relative
		; to the current character count, not byte count. Because there are two bytes per character, we
		; have to multiply _CurX64 by 2 first, then add it to our screen width * y.
		;--------------------------------

		MOV	    al, byte [_CurX64]	; multiply _CurX64 by 2 because it is 2 bytes per char
		
        MOV	    cl, 2
		
        MUL	    cl
		
        POP	    rcx			        ; pop y*COLS result
		
        ADD	    rax, rcx

		;-------------------------------
		; Now eax contains the offset address to draw the character at, so just add it to the base address
		; of video memory (Stored in edi)
		;-------------------------------

		XOR	    rcx, rcx
		
        ADD	    rdi, rax		    ; add it to the base address

	;-------------------------------;
	;   Watch for new line          ;
	;-------------------------------;

	CMP	    bl, 0x0A		        ; is it a newline character?
	
    JE	    .Row			        ; yep--go to next row

	;-------------------------------;
	;   Print a character           ;
	;-------------------------------;

	MOV	    dl, bl			        ; Get character
	
    MOV	    dh, CHAR_ATTRIB		    ; the character attribute
	
    MOV	    word [rdi], dx		    ; write to video display

	;-------------------------------;
	;   Update next position        ;
	;-------------------------------;

	INC	    byte [_CurX64]		    ; go to next character

;	CMP	    byte [_CurX64], COLS    ; are we at the end of the line?

;	JE	    .Row			        ; yep-go to next row

	JMP	    .done			        ; nope, bail out

	;-------------------------------;
	;   Go to next row              ;
	;-------------------------------;

.Row:
    MOV	    byte [_CurX64], 0		; go back to col 0
	
    INC	    byte [_CurY64]		    ; go to next row

	;-------------------------------;
	;   Restore registers & return  ;
	;-------------------------------;

.done:
	RESTORE_STATE				    ; restore registers and return
	
    RET

;**************************************************;
;	Puts64 ()
;		- Prints a null terminated string
;	parm\ EBX = address of string to print
;**************************************************;

Puts64:
	;-------------------------------;
	;   Store registers             ;
	;-------------------------------;
	SAVE_STATE				        ; save registers
	
    PUSH	rbx			            ; copy the string address
	
    POP	    rdi

.loop:

	;-------------------------------;
	;   Get character               ;
	;-------------------------------;
	MOV	    bl, byte [edi]		    ; get next character
	
    CMP	    bl, 0			        ; is it 0 (Null terminator)?
	
    JE	    .done			        ; yep-bail out

	;-------------------------------;
	;   Print the character         ;
	;-------------------------------;

	CALL	Putch64			        ; Nope-print it out

	;-------------------------------;
	;   Go to next character        ;
	;-------------------------------;

	INC	    rdi			            ; go to next character
	JMP	    .loop

.done:

	;-------------------------------;
	;   Update hardware cursor      ;
	;-------------------------------;

	; Its more efficiant to update the cursor after displaying
	; the complete string because direct VGA is slow

	MOV	    bh, byte [_CurY64]	    ; get current position
	
    MOV	    bl, byte [_CurX64]
	
    CALL	MovCur64			    ; update cursor

	RESTORE_STATE				    ; restore registers, and return
	
    RET

;**************************************************;
;	MoveCur ()
;		- Update hardware cursor
;	parm/ bh = Y pos
;	parm/ bl = x pos
;**************************************************;

bits 64

MovCur64:

	SAVE_STATE				        ; save registers (aren't you getting tired of this comment?)

	;-------------------------------;
	;   Get current position        ;
	;-------------------------------;

	; Here, _CurX64 and _CurY64 are relitave to the current position on screen, not in memory.
	; That is, we don't need to worry about the byte alignment we do when displaying characters,
	; so just follow the forumla: location = _CurX64 + _CurY64 * COLS

	XOR	    rax, rax
	
    MOV	    rcx, COLS
	
    MOV	    al, bh			        ; get y pos
	
    MUL	    rcx			            ; multiply y*COLS
	
    ADD	    al, bl			        ; Now add x
	
    MOV	    rbx, rax

	;--------------------------------------;
	;   Set low byte index to VGA register ;
	;--------------------------------------;

	MOV	    al, 0x0f
	
    MOV	    dx, 0x03D4
	
    OUT	    dx, al

	MOV	    al, bl
	
    MOV	    dx, 0x03D5
	
    OUT	    dx, al			        ; low byte

	;---------------------------------------;
	;   Set high byte index to VGA register ;
	;---------------------------------------;

	XOR	rax, rax

	MOV	al, 0x0e
	
    MOV	dx, 0x03D4
	
    OUT	dx, al

	MOV	al, bh
	
    MOV	dx, 0x03D5
	
    OUT	dx, al			            ; high byte

	RESTORE_STATE
	
    RET

;**************************************************;
;	ClrScr64 ()
;		- Clears screen
;**************************************************;

bits 64

ClrScr64:

	SAVE_STATE
	
    CLD
	
    MOV	    rdi, VIDMEM 	;0xb8000
	
    MOV	    rcx, 2000
	
    MOV	    ah, CHAR_ATTRIB
	
    MOV	    al, ' '
	
    REP	    stosw

	MOV	    byte [_CurX64], 0
	
    MOV	    byte [_CurY64], 0
	
    RESTORE_STATE
	
    RET

;**************************************************;
;	GotoXY64 ()
;		- Set current X/Y location
;	parm\	AL=X position
;	parm\	AH=Y position
;**************************************************;

bits 64

GotoXY64:
	SAVE_STATE
	
    MOV	    [_CurX64], al		        ; just set the current position
	
    MOV	    [_CurY64], ah
	
    RESTORE_STATE
	
    RET




%endif ; __ASMLIB64_INC