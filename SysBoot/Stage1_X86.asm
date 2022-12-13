; Calling convention helpers
%macro STACK_FRAME_BEGIN 0
    push bp
    mov bp, sp
%endmacro
%macro STACK_FRAME_END 0
    mov sp, bp
    pop bp
%endmacro

ImageLoadSeg EQU 60h     ; <=07Fh because of "push byte ImageLoadSeg" instructions

%define ARG0 [bp + 4]
%define ARG1 [bp + 6]
%define ARG2 [bp + 8]
%define ARG3 [bp + 10]
%define ARG4 [bp + 12]
%define ARG5 [bp + 14]

%define LVAR0 word [bp - 2]
%define LVAR1 word [bp - 4]
%define LVAR2 word [bp - 6]
%define LVAR3 word [bp - 8]

; *************************
;    Real Mode 16-Bit
; - Uses the native Segment:offset memory model
; - Limited to 1MB of memory
; - No memory protection or virtual memory
; *************************
BITS	16							; We are in 16 bit Real Mode

ORG		0x7C00						; We are loaded by BIOS at 0x7C00

MAIN:       
    JMP Entry              ; Jump over the Fat32 Blocks
    nop

; *************************
; FAT Boot Parameter Block
; *************************
BS_OEMName					db		"MSWIN4.1"
BPB_BytsPerSec				dw		0
BPB_SecPerClus  			db		0
BPB_RsvdSecCnt  			dw		0
BPB_NumFATs					db		0
BPB_RootEntCnt				dw		0
BPB_TotSec16				dw		0
BPB_Media					db		0
BPB_FatSz16 				dw		0
BPB_SecPerTrk   			dw		0
BPB_NumHeads    			dw		0
BPB_HiddSec 				dd 		0
BPB_TotSec32				dd 		0

; *************************
; FAT32 Extension Block
; *************************
BPB_FATSz32     			dd 		0
BPB_ExtFlags				dw		0
BPB_FSVer					dw		0
BPB_RootClus				dd 		0
BPB_FSInfo  				dw		0
BPB_BkBootSec   			dw		0

; Reserved 
dReserved0					dd		0 	;FirstDataSector

BS_DrvNum       			db		0

dReserved1					dd		0 	;ReadCluster

BS_BootSig  				db		0
dReserved2					dd 		0 	;ReadCluster

bReserved3					db		0

BS_VolID    				dd 		0

BS_VolLab   				db		"NO NAME    "

BS_FilSysType				db		"FAT32   "

;***************************************
;	Prints a string
;	DS=>SI: 0 terminated string
;***************************************

PRINT16BIT:
			lodsb					        ; load next byte from string from SI to AL

			or			al, al		        ; Does AL=0?
			
            jz			PRINTDONE16BIT	    ; Yep, null terminator found-bail out
			
            mov			ah,	0eh	            ; Nope-Print the character
			
            int			10h
			
            jmp			PRINT16BIT		    ; Repeat until null terminator found

PRINTDONE16BIT:
			
            ret					            ; we are done, so return

;*************************************************;
;	Bootloader Entry Point
;*************************************************;
; dl = drive number
; si = partition table entry
Entry:
    cli ;disable interrupts
    jmp 0:loadCS

loadCS:
    cli                        ; Disable Interrupts
    
    mov ax, 0x0000             ; Set ax to 0

    mov ds, ax                 ; Set segment ds to 0
    
    mov es, ax                 ; Set segment es to 0
    
    mov ss, ax                 ; Set segment ss to 0
    
    mov sp, 0x7C00             ; Set stack-pointer to 0

checkStack:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;; How much RAM is there? ;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    int 12h                    ; Get size in KB
    
    shl ax, 6                  ; convert it to 16-byte paragraphs

    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;; Reserve memory for the boot sector and its stack ;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    sub ax, 512/16             ; reserve 512 bytes for the bootsector

    mov es, ax                 ; es:0 -> top - 512

    sub     ax, 2048 / 16      ; reserve 2048 bytes for the stack
    
    mov     ss, ax             ; ss:0 -> top - 512 - 2048
    
    mov     sp, 2048           ; 2048 bytes for the stack

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;; Copy ourselves to top of memory ;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov     cx, 256

    mov     si, 7C00h
    
    xor     di, di
    
    mov     ds, di
    
    rep     movsw

    ;;;;;;;;;;;;;;;;;;;;;;
    ;; Jump to the copy ;;
    ;;;;;;;;;;;;;;;;;;;;;;

    push    es
    
    push    byte Stage1
    

Stage1:
    push cs

    pop ds

    mov byte [BS_DrvNum], dl

    and     byte [BPB_RootClus+3], 0Fh ; mask cluster value
    
    mov     esi, [BPB_RootClus] ; esi=cluster # of root dir



times 510-($-$$) DB 0
dw 0xAA55