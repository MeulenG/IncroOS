; ASM = Right to left execution
bits	16							; We are still in 16 bit Real Mode

org		0x7c00						; We are loaded by BIOS at 0x7C00

start:          jmp Main					; jump over FAT block
; *************************
; FAT Boot Parameter Block
; *************************
szOemName					db		"MY    OS"
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

Print16bitmode:
			lodsb					; load next byte from string from SI to AL
			or			al, al		; Does AL=0?
			jz			PrintDone16bitmode	; Yep, null terminator found-bail out
			mov			ah,	0eh	; Nope-Print the character
			int			10h
			jmp			PrintDone16bitmode		; Repeat until null terminator found
PrintDone16bitmode:
			ret					; we are done, so return

;*************************************************;
;	Bootloader Entry Point
;*************************************************;
	; We are in 16-bit Real Mode, therefore we use the 16-bit registers : CS, DS, ES, FS, GS, and SS.
Main:
	xor	ax, ax		; Setup segments to insure they are 0. Remember that
	mov	ds, ax		; we have ORG 0x7c00. This means all addresses are based
	mov	es, ax		; from 0x7c00:0. Because the data segments are within the same
					; code segment, null em.

	xor	ax, ax		; clear ax
	int	0x12		; get the amount of KB from the BIOS

	mov sp,0x7c00	; Set the Stack	


TestDiskExtension:
    mov [DriveId],dl
    mov ah,0x41
    mov bx,0x55aa
    int 0x13
    jc Stage1Error
    cmp bx,0xaa55
    jne Stage1Error

LoadLoader:
    mov si,ReadPacket
    mov word[si],0x10
    mov word[si+2],5
    mov word[si+4],0x7e00
    mov word[si+6],0
    mov dword[si+8],1
    mov dword[si+0xc],0
    mov dl,[DriveId]
    mov ah,0x42
    int 0x13
    jc  ReadStage1Error

    mov dl,[DriveId]
    jmp 0x7e00 

ReadStage1Error:
Stage1Error:
    mov si, msg
    call Print16bit

End:
    hlt    
    jmp End

msg	db	"Error at Stage1", 0		; the string to print
ReadPacket: times 16 db 0

times 510 - ($-$$) db 0						; We have to be 512 bytes. Clear the rest of the bytes with 0

dw 0xAA55							; Boot Signiture