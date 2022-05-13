bits	16							; We are still in 16 bit Real Mode

ORG		0x7c00						; We are loaded by BIOS at 0x7C00

main:       jmp start               ; Jump over the Fat32 Blocks

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

Print16bit:
			lodsb					; load next byte from string from SI to AL
			or			al, al		; Does AL=0?
			jz			PrintDone16bit	; Yep, null terminator found-bail out
			mov			ah,	0eh	; Nope-Print the character
			int			10h
			jmp			Print16bit		; Repeat until null terminator found
PrintDone16bit:
			ret					; we are done, so return

;*************************************************;
;	Bootloader Entry Point
;*************************************************;

start:
    xor ax,ax   
    mov ds,ax
    mov es,ax  
    mov ss,ax
    mov sp,0x7c00

TestDiskExtension:
    mov [DriveId],dl
    mov ah,0x41
    mov bx,0x55aa
    int 0x13
    jc NotSupport
    cmp bx,0xaa55
    jne NotSupport

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
    jc  ReadError

    mov dl,[DriveId]
    jmp 0x7e00 

ReadError:
NotSupport:
    mov si, msg
    call Print16bit

End:
    hlt    
    jmp End

;*******************************************************
;	Data Section
;*******************************************************
msg	db	"Hahaha, Fuck you, but at stage1", 0
MessageLenmsg: equ $-msg
ReadPacket: times 16 db 0
msgStart     db 0x0D,0x0A, "Boot Loader starting (0x7C00)....", 0x0D, 0x0A, 0x00
MessageLenmsgStart: equ $-msgStart
msgStageTwo  db "Jumping to stage2 (0x7E00)", 0x0D, 0x0A, 0x0D, 0x0A, 0x00
MessageLenmsgStageTwo: equ $-msgStageTwo

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
