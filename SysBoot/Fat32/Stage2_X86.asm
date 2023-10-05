; *************************
; General x86 Real Mode Memory Map:
;   - 0x00000000 - 0x000003FF - Real Mode Interrupt Vector Table
;   - 0x00000400 - 0x000004FF - BIOS Data Area
;   - 0x00000500 - 0x00007BFF - Unused
;   - 0x00007C00 - 0x00007DFF - Stage 1 (512 Bytes)
;   - 0x00007E00 - 0x0009FFFF - Stage 2
;   - 0x000A0000 - 0x000BFFFF - Video RAM (VRAM) Memory
;   - 0x000B0000 - 0x000B7777 - Monochrome Video Memory
;   - 0x000B8000 - 0x000BFFFF - Color Video Memory
;   - 0x000C0000 - 0x000C7FFF - Video ROM BIOS
;   - 0x000C8000 - 0x000EFFFF - BIOS Shadow Area
;   - 0x000F0000 - 0x000FFFFF - System BIOS
; *************************
; *************************
;    Real Mode 16-Bit
; - Uses the native Segment:offset memory model
; - Limited to 1MB of memory
; - No memory protection or virtual memory
; *************************
BITS	16

ORG     0x7E00
main_Stage2: JMP Stage2_Main

;*******************************************************
;	Preprocessor directives 16-BIT MODE
;*******************************************************
%include "../Routines/stdio16.inc"
%include "../Routines/Gdt.inc"
%include "../Routines/Idt.inc"

;*******************************************************
;	Preprocessor A20
;*******************************************************
%include "../Routines/A20.inc"

;******************************************************
;	ENTRY POINT For STAGE 2
;******************************************************
Stage2_Main:
	; Clear Interrupts
    CLI
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    XOR     ax, ax
    
    MOV	    ds, ax
    
    MOV	    es, ax
    
    MOV	    fs, ax
    
    MOV	    gs, ax

    MOV	    ss, ax
    
    MOV	    ax, 0x7C00
    
    MOV	    sp, ax
    ; Enable Interrupts
    STI
    ; Save Drive Number in DL
    MOV     [bPhysicalDriveNum],dl
   
	CALL FixCS

;Most Modern Computers already have the A20 line set from the get-go, but if not then we enable it
SetA20:
    ;-------------------------------;
	;   Enable A20 Line		        ;
	;-------------------------------;
    CALL EnableA20_KKbrd_Out

SetVideoMode:
    ;-------------------------------;
	;   Set Video Mode  	        ;
	;-------------------------------;
    MOV     ax, 3

    INT     0x10
    
    CLI
    ;-------------------------------;
	;   Install our GDT		        ;
	;-------------------------------;
    call    GdtInstall
    
    ;-------------------------------;
	;   Install our IDT		        ;
	;-------------------------------;
    LIDT    [idt_real]
    
	;-------------------------------;
	;   Get Ready For PMode		    ;
	;-------------------------------;
    MOV     eax,cr0             ; Set the A-register to control register 0.
    
    OR  eax, (1 << 0)           ; Set The PM-bit, which is the 0th bit.
    
    MOV     cr0,eax             ; Set control register 0 to the A-register.

    JMP 8:ProtectedMode_Stage3  ; Get outta this cursed Real Mode
ALIGN   32
BITS    32
;*******************************************************
;	Preprocessor directives 32-BIT MODE
;*******************************************************
%include "../Routines/stdio32.inc"
;******************************************************
;	ENTRY POINT For STAGE 3
;******************************************************
ProtectedMode_Stage3:
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    MOV     ax, 0x20
    
    MOV     ds, ax
    
    MOV     es, ax
    
    MOV     ss, ax
    
    MOV     esp, 0x7c00

    JMP     8:Stage4_Long_Mode

PEnd:
    HLT
    
    JMP     PEnd

ALIGN   64
BITS    64
;*******************************************************
;	Preprocessor directives 64-BIT MODE
;*******************************************************
%include "../Routines/stdio64.inc"
;******************************************************
;	ENTRY POINT For STAGE 4
;******************************************************
Stage4_Long_Mode:
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    xor rax, rax
    mov ax, 0x30
    mov ds, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov es, ax

	jmp 0xA000

    
LEnd:
    HLT
    
    JMP     LEnd