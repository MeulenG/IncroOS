; Calling convention helpers
%macro STACK_FRAME_BEGIN 0
    push bp
    mov bp, sp
%endmacro
%macro STACK_FRAME_END 0
    mov sp, bp
    pop bp
%endmacro

IMGADDR     equ 0x60

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
    cli ; Disable Interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    push ax

Stage1:
    mov dl, byte [BS_DrvNum] ; DL = Drive Number
    
    ; Lets calculate the FirstDataSector = BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors(Its zero) + Partitiontable
    mov di, PartitionTableData
    mov cx, 0x0008 ; 8 words = 16 bytes
    repnz movsw

    ; clear eax and ebx
    xor eax, eax
    xor ebx, ebx
    
    movzx eax, byte [BPB_NumFATs] ; (BPB_NumFATs * FATSz)
    mov ebx, dword [BPB_FATSz32]
    imul eax, ebx
    
    xor ebx, ebx ; clear it from previous value
    mov ebx, dword [BPB_RsvdSecCnt] ; BPB_ResvdSecCnt + (BPB_NumFATs * FATSz)
    add eax, ebx

    xor ebx, ebx
    mov ebx, dword [dBaseSector]
    add eax, ebx ; BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + Partitiontable

    ; save result
    push eax
    pop eax

find_File:
    xchg bx, bx
    mov dx, word [BPB_RootClus]
    push di

search_Root_Files:
    push di
    mov si, FileName
    mov cx, 11
    rep cmpsb
    pop di
    je read_File

    add di, 32
    dec dx

    call ColdReboot
        

read_File:
    push di
    push es
    pop ds
    pop si

    mov di, 0x7E00
    mov es, di

    pop ax
    call FirstSectorofCluster

    jmp 0x0:0x7E00

FirstSectorofCluster:
    ; FirstSectorofCluster = ((N – 2) * BPB_SecPerClus) + FirstDataSector;
    ; preserve registers
    pushad
    
    lea   edi, [esi-2]
    movzx   eax, byte [BPB_SecPerClus]
    imul  edi, eax

    movzx eax, byte [BPB_NumFATs]
    imul  eax, [BPB_FATSz32]
    add   edi, eax

    movzx eax, word [BPB_RsvdSecCnt]
    add   eax, edi
    add   eax, [BPB_HiddSec]
        
    push  dx
    mov   cx, [BPB_SecPerClus]

    ret

Disk_Error:
    mov si, Disk_Error
    call PRINT16BIT

ColdReboot:
    mov si, HALT_MSG
    call PRINT16BIT
    cli
    hlt

;*************************************************;
;   TODO
; 1) Fix the stack mistakes
; 2) Properly read file
; 3) Test
;*************************************************;



;*************************************************;
;   Global Variables
;*************************************************;
PartitionTableData dq 0
dBaseSector       dd 0

dSectorCount      dd 0

FileName   db "STAGE2  SYS"
;DAP - Disk Address Packet, Used by INT13h Extensions	
disk_Address_Packet:
    DAP_Size					db 0x10			;Size of packet, always 16 BYTES
    DAP_Reserved				db 0x00			;Reserved
    DAP_Sectors					dw 0x0000		;Number of sectors to read
    DAP_Offset					dw 0x0000		;Offset to read data to
    DAP_Segment 				dw 0x0000		;Segment to read data to
    DAP_LBA_LSD					dd 0x00000000	;QWORD with LBA of where to start reading from
    DAP_LBA_MSD					dd 0x00000000
;*************************************************;
;   Error Messages
;*************************************************;
DISK_ERROR	db "Error reading from disk!",0
HALT_MSG	db 0x0D,0x0A,"Halting machine. Power off and reboot.",0


;*************************************************;
;   Data
;*************************************************;
loadFileError db "Can't load file"
findFileError db "Unable to find your file"

times 510-($-$$) DB 0
dw 0xAA55