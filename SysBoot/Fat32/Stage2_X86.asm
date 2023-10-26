; *************************
; General x86 Real Mode Memory Map:
;   - 0x00000000 - 0x000003FF - Real Mode Interrupt Vector Table
;   - 0x00000400 - 0x000004FF - BIOS Data Area
;   - 0x00000500 - 0x00007BFF - Kernel
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
bits	16

org     0x7E00
jmp LoaderEntry16

;*******************************************************
;	Preprocessor directives 16-BIT MODE
;*******************************************************
%include "../includes/stdio16.inc"
%include "../includes/Macros.inc"
%include "../includes/KernelLoader.inc"
%include "../includes/GlobalDefines.inc"
%include "../includes/cpu.inc"
%include "../includes/Gdt.inc"
%include "../includes/Idt.inc"
%include "../includes/A20.inc"
%include "../includes/Common16.inc"

;*******************************************************
;	Data Section
;*******************************************************
%include "../includes/DataSection.inc"

;*******************************************************
;	STAGE 2 ENTRY POINT
;
;		-Store BIOS information
;		-Load Kernel
;		-Install GDT; go into protected mode (pmode)
;		-Jump to Stage 3
;*******************************************************
LoaderEntry16:
    call    SetStack16

Continue_Part1:
    ; Save Drive Number in DL
    mov     [bPhysicalDriveNum],dl
    ; Is this CPU eligible?
    call    DetectCPU
    ; Let's load our kernel
	call    FixCS

;Most Modern Computers already have the A20 line set from the get-go, but if not then we enable it
SetA20:
    ;-------------------------------;
	;   Enable A20 Line		        ;
	;-------------------------------;
    call    EnableA20_KKbrd_Out

SetVideoMode:
    ;-------------------------------;
	;   Set Video Mode  	        ;
	;-------------------------------;
    mov     ax, 3
    int     0x10
    cli
    ;-------------------------------;
	;   Install our GDT		        ;
	;-------------------------------;
    call    GdtInstall
    
    ;-------------------------------;
	;   Install our IDT		        ;
	;-------------------------------;
    call    InstallIDT
    
	;-------------------------------;
	;   Get Ready For PMode		    ;
	;-------------------------------;
    mov     eax,cr0             ; Set the A-register to control register 0.
    
    OR      eax, (1 << 0)       ; Set The PM-bit, which is the 0th bit.
        
    mov     cr0,eax             ; Set control register 0 to the A-register.

    jmp     CODE_DESC:LoaderEntry32  ; Get outta this cursed Real Mode
ALIGN   32
BITS    32
;*******************************************************
;	Preprocessor directives 32-BIT MODE
;*******************************************************
%include "../includes/stdio32.inc"
%include "../includes/Paging.inc"
%include "../includes/Common32.inc"
;******************************************************
;	ENTRY POINT For STAGE 3
;******************************************************
LoaderEntry32:
    call    SetStack32

Continue_Part2:
    call    PMode_Setup_Paging
    jmp     CODE64_DESC:LoaderEntry64

ALIGN   64
BITS    64
;*******************************************************
;	Preprocessor directives 64-BIT MODE
;*******************************************************
%include "../includes/stdio64.inc"
%include "../includes/Common64.inc"
;******************************************************
;	ENTRY POINT For STAGE 4
;******************************************************
LoaderEntry64:
    xchg bx, bx
    call    SetStack64

Continue_Part3:
	jmp     KERNEL_BASE_ADDRESS