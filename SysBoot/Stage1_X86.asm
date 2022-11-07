; *************************
;    Real Mode 16-Bit
; - Uses the native Segment:offset memory model
; - Limited to 1MB of memory
; - No memory protection or virtual memory
; *************************
BITS	16							; We are in 16 bit Real Mode

ORG		0x7C00						; We are loaded by BIOS at 0x7C00

MAIN:       JMP Entry              ; Jump over the Fat32 Blocks

; *************************
; FAT Boot Parameter Block
; *************************
szOemName					DB		"MY    OS"
BPB_BytesPerSec				DW		257   ; Bytes Per Sector ; Offset 0x0B ; 16 Bits ; Value: Always 512 Bytes
BPB_SecPerClus  			DB		1   ; Sectors Per Cluster ; Offset 0x0D ; 8 Bits ; Value: 1, 2, 4, 8, 16, 32, 64, 128
BPB_RsvdSecCnt  			DW		257   ; Number of Reserved Sectors ; Offset 0x0E ; 16 Bits ; Value: 0x20
BPB_NumFATs					DB		1   ; Number of FATs ; Offset 0x10 ; 8 Bits ; Value: Always 2
wRootEntries				DW		0
BPB_TotSec32				DD		16843009 ; Total Sectors
BPB_Media					DB		1
BPB_FATSz32 				DD		16843009   ; Sectors Per FAT ; Offset 0x24 ; 32 Bits ; Depends On Disk Size
BPB_SecPerTrk   			DW		257
BPB_NumHeads    			DW		257
BPB_HiddSec 				DD 		16843009
dTotalSectors				DD 		0

; *************************
; FAT32 Extension Block
; *************************
dSectorsPerFat32			DD 		0
wFlags						DW		0
wVersion					DW		0
BPB_RootClus				DD 		16843009   ; Root Directory First Cluster ; Offset 0x2C ; 32 Bits ; Value: 0x00000002
wFSInfoSector				DW		0
wBackupBootSector			DW		0

; Reserved 
dReserved0					DD		0 	;FirstDataSector
dReserved1					DD		0 	;ReadCluster
dReserved2					DD 		0 	;ReadCluster

bPhysicalDriveNum			DB		0
bReserved3					DB		0
bBootSignature				DB		0
dVolumeSerial				DD 		0
szVolumeLabel				DB		"NO NAME    "
szFSName					DB		"FAT32   "

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
    xchg bx, bx
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
    cld



ReadSectors:
    mov bp, sp
    push dx
    
    ; Lets calculate the first data sector = bNumFATs * BPB_FATSZ32 + BPB_ResvdSecCnt
    mov al, [BPB_NumFATs]
    mov ebx, [BPB_FATSz32]
    mul ebx
    xor ebx, ebx
    mov bx, [BPB_RsvdSecCnt]
    add eax, ebx
    ; Our ax register is our first data sector
    push eax

    ; We need to get the location of the first FAT
    ; FATsector = BPB_ResvdSecCnt
    xor eax, eax
    mov ax, [BPB_RsvdSecCnt]
    push eax

    ; BytsPerCluster = BPB_BytsPerSec * BPB_SecPerClus
    mov ax, [BPB_BytesPerSec]
    mov bx, [BPB_SecPerClus]
    mul bx
    push eax
    
    ; FATClusterMask = 0x0FFFFFFF
    mov eax, 0x0FFFFFFF
    push eax
	and al, 0xF8
	push eax

    ; CurrentCluster = BPB_RootClus
    mov eax, [BPB_RootClus]
    push eax

ImageName   db "KRNLDR  SYS"
times 510-($-$$) DB 0
dw 0xAA55