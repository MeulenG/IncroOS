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
szOemName					db		"My    OS"
wBytesPerSector				dw		0
bSectorsPerCluster			db		0
wReservedSectors			dw		0
bNumFATs					db		0
wRootEntries				dw		0
wTotalSectors				dw		0
bMediaType					db		0
wSectorsPerFat				dw		0
wSectorsPerTrack			dw		0
wHeadsPerCylinder			dw		0
dHiddenSectors				dd 		0
dTotalSectors				dd 		0

; *************************
; FAT32 Extension Block
; *************************
dSectorsPerFat32			dd 		0
wFlags						dw		0
wVersion					dw		0
dRootDirStart				dd 		0
wFSInfoSector				dw		0
wBackupBootSector			dw		0

; Reserved 
dReserved0					dd		0 	;FirstDataSector
dReserved1					dd		0 	;ReadCluster
dReserved2					dd 		0 	;ReadCluster

bPhysicalDriveNum			db		0
bReserved3					db		0
bBootSignature				db		0
dVolumeSerial				dd 		0
szVolumeLabel				db		"NO NAME    "
szFSName					db		"FAT32   "

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
    xor ax, ax
    mov es, ax
    mov ss, ax

    ; we setup our stack, this is temporary for our stage1(512 bytes)
    mov sp, 0x7C00
    mov sp, ax
    sti
    
    ; Save drive number in dl
    mov byte[bPhysicalDriveNum], dl

    ; Lets calculate the first data sector = bNumFATs * dSectorsPerFat32 + wReservedSectors + Partitiontable 
    mov di, PartitionTableData    
    mov cx, 0x0008 ; 8 words = 16 bytes
    repnz movsw
    ; clear ax
    xor ax, ax
    mov al, byte [bNumFATs]
    mov bx, dword [dSectorsPerFat32]
    mul bx
    mov ax, word [wReservedSectors]
    add ax, bx
    mov dword [dReserved0], ax
    ; save result
    push ax


FindFile:
    mov cx, 11

findFileLoop:
    cmp byte [es : di], ch
    je findFileError

    pusha
    repe cmpsb
    popa
    je .findFileLoopFound

    add di, 32
    dec dx
    jnz .findFileLoop
    popf
    pop esi

    jc readRootDir
    jmp .findFileError

fNameFound:
    push word [es : di + 0x14]
    push word [es : di + 0x1A]

    pop esi

    pop eax
    pop es
    jmp loadFile16


loadFile16:


findFileError:


findFileLoop
; IN (es:bx is DestinationBuffer, esi is ClusterNumber!)
ReadCluster:
    ; We calculate our Fat32 data sector number
    ; bpbHiddenSectors + bpbReservedSectors + bpbNumberOfFATs * bsSectorsPerFAT32 + (N - 2) * bpbSectorsPerCluster
    lea edi, [esi-2]
    mov eax, byte [bSectorsPerCluster]
    imul edi, eax

    movzx eax, byte [bNumFATs]
    imul eax, dword [dSectorsPerFat32]
    add edi, eax

    movzx eax, edi
    add eax, edi
    add eax, dword [dHiddenSectors]

    push dx
    mov cx, 1
    call ReadSectorLBA

    mov ax, word [wBytesPerSector]
    shr ax, 4
    mul cx
    
    mov cx, es
    add cx, ax
    mov es, cx
    cmp esi, 0x0FFFFFF8
    ret

ReadSectorLBA:
    ; save states
    pushad
;*************************************************;
;   Global Variables
;*************************************************;
PartitionTableData dq 0
FileName   db "STAGE2  SYS"


;*************************************************;
;   Data
;*************************************************;
loadFileError db "Can't load file"
findFileError db "Unable to find your file"

times 510-($-$$) DB 0
dw 0xAA55