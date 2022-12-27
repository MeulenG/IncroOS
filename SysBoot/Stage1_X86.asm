; 16 Bit Code, Origin at 0x0
BITS 16
ORG 0x7C00


; Jump Code, 3 Bytes
jmp short Main
nop

; *************************
; FAT Boot Parameter Block
; *************************
szOemName					db		"OMOS    "
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


; *************************
; Bootloader Entry Point
; *************************

Main:
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

    
; *************************
; Global Variables
; *************************
DefStage2	db 	"STAGE2  SYS"
Stage1_JMP_Message db "Jumping to 0x7E00"

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