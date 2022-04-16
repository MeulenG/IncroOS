;   General x86 Real Mode Memory Map:
;   0x00000000 - 0x000003FF - Real Mode Interrupt Vector Table
;   0x00000400 - 0x000004FF - BIOS Data Area
;   0x00000500 - 0x00007BFF - Unused
;   0x00007C00 - 0x00007DFF - Our Bootloader
;   0x00007E00 - 0x0009FFFF - Unused
;   0x000A0000 - 0x000BFFFF - Video RAM (VRAM) Memory
;   0x000B0000 - 0x000B7777 - Monochrome Video Memory
;   0x000B8000 - 0x000BFFFF - Color Video Memory
;   0x000C0000 - 0x000C7FFF - Video ROM BIOS
;   0x000C8000 - 0x000EFFFF - BIOS Shadow Area
;   0x000F0000 - 0x000FFFFF - System BIOS
;   ASM = Right to left execution
;   I'm gonna have to make this code mega ultra idiot proof in-case i have to look at it more, sorry in advance haha
bits	16							; We are loaded in 16-bit Real Mode

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
    mov ah,0x13
    mov al,1
    mov bx,0xa
    xor dx,dx
    mov bp,msgStart
    mov cx,MessageLenmsgStart
    int 0x10

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
    mov ah,0x13
    mov al,1
    mov bx,0xa
    xor dx,dx
    mov bp,msgStageTwo
    mov cx,MessageLenmsgStageTwo
    int 0x10
    jmp 0x7e00 

ReadStage1Error:
Stage1Error:
    mov ah,0x13
    mov al,1
    mov bx,0xa
    xor dx,dx
    mov bp,msg
    mov cx,MessageLenmsg
    int 0x10

End:
    hlt    
    jmp End


;*******************************************************
;	Data Section
;*******************************************************
DriveId:    db 0
msg	db	"Error at Stage1", 0
MessageLenmsg: equ $-msg
ReadPacket: times 16 db 0
msgStart     db 0x0D,0x0A, "Boot Loader starting (0x7C00)....", 0x0D, 0x0A, 0x00
MessageLenmsgStart: equ $-msgStart
msgStageTwo  db "Jumping to stage2 (0x7E00)", 0x0D, 0x0A, 0x0D, 0x0A, 0x00
MessageLenmsgStageTwo: equ $-msgStageTwo
;*******************************************************
;   Boot Signature
;*******************************************************
times (0x1be-($-$$)) db 0

    db 80h
    db 0,2,0
    db 0f0h
    db 0ffh,0ffh,0ffh
    dd 1
    dd (20*16*63-1)
	
    times (16*3) db 0

    db 0x55
    db 0xaa