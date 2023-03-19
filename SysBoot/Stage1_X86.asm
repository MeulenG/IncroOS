; 16 Bit Code, Origin at 0x0
BITS 16
ORG 0x7C00


; Jump Code, 3 Bytes
jmp short Main
nop

; *************************
; FAT Boot Parameter Block
; *************************
szOemName					db		"Incro-OS"
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

; *************************
; Bootloader Entry Point
; *************************

Main:
    xchg bx, bx
    cli ;Disable Interrupts

    jmp 0x0:FixStack ; Let's fix segments and the stack

FixStack:
    xor ax, ax ; Set ax to 0
    mov ds, ax ; Move 0 to ds
    mov es, ax ; Move 0 to es
    mov ss, ax ; Move 0 to ss

    ; Set the Stack
    mov ax, 0x7C00
    mov sp, ax

    ; Segments and stack fixed, lets enable interrupts again
    sti

    ; Save Drive Num
    mov BYTE [bPhysicalDriveNum], dl

    ; Calculate the First Data Cluster. FirstDataSector = BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors
    ; RootDirSectors is always 0
    xor eax, eax ; Set eax to 0
    mov al, BYTE [bNumFATs] ; Move BPB_NumFATs(bNumFATs) into al
    mov ebx, DWORD [dSectorsPerFat32] ; Move FATSz(dSectorsPerFat32) into ebx, since it is a dword
    mul ebx ; Multiply BPB_NumFATs(bNumFATs) with FATSz(dSectorsPerFat32)
    mov bx, WORD [wReservedSectors] ; Move BPB_ResvdSecCnt(wReservedSectors) into bx
    add eax, ebx ; Add BPB_ResvdSecCnt with the result of (BPB_NumFATs * FATSz)

    ; Calculate the root directory
    mov eax, DWORD[dRootDirStart] ; FirstDataSector
    mov ebx, DWORD[bSectorsPerCluster]
    mul ebx
    add eax, DWORD[wReservedSectors]
    add eax, DWORD[wTotalSectors]

    ; Now we calculate the LBA = ((RootDirSector - 2) * BPB_SecPerClus) + FirstDataSector
    mov eax, [dRootDirStart]
    mov ebx, [bSectorsPerCluster]
    sub eax, 2
    mul ebx
    add eax, [dReserved0]
    mov [DRootLBA], eax

ReadSector:
    .MAIN
        mov di, 0x0005 ; five retries for error
    .SECTORREADLOOP
        pusha
        mov ah, 0x02 ; BIOS read sector
        mov al, 0x01 ; Read one sector
        mov ch, BYTE[wHeadsPerCylinder]
        mov dh, BYTE[wSectorsPerTrack]
        mov dl, BYTE[bPhysicalDriveNum]
        mov cl, BYTE[wSectorsPerFat]
        int 0x13
        xor ax, ax
        int 0x13
        dec di
        popa
        int 0x18
    .SUCESS
        mov si, DefStage2
        call PRINT16BIT
        popa
        add bx, WORD[wBytesPerSector]
        inc ax
        loop .MAIN
        ret
        
; *************************
; Global Variables
; *************************
DefStage2	db 	"STAGE2  SYS"
Stage1_JMP_Message db "Jumping to 0x7E00"
DRootLBA dd 0
; *************************
; Error Codes
; *************************


; *************************
; Fill Out The Bootloader
; *************************
times 510-($-$$) db 0

; *************************
; Boot Signature
; *************************
db 0x55, 0xAA