; 16 Bit Code, Origin at 0x0
BITS 16
ORG 0x7C00


%macro STACK_FRAME_BEGIN16 0
    push bp
    mov bp, sp
%endmacro

%macro STACK_FRAME_END16 0
    mov sp, bp
    pop bp
%endmacro

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

%include "asmlib16.inc"
Main:
    ;-------------------------------------------------------
    ; Let's set the stack first thing
    ;-------------------------------------------------------
    xchg    bx, bx

    cli

    xor     ax,ax

    mov     ds,ax

    mov     es,ax

    ;--------------------------------------------------------
    ; Set the stack pointer
    ;--------------------------------------------------------

    mov     ss,ax

    mov     sp,0x7c00

    sti

    cld

    ;--------------------------------------------------------
    ; Let's display a loading message
    ;--------------------------------------------------------
    mov     si, MsrLoading
    call    Puts16

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
    mov ah,0x13

    mov al,1

    mov bx,0xa

    xor dx,dx

    mov bp,Message

    mov cx,MessageLen

    int 0x10

End:
    hlt

    jmp End


;--------------------------------------------------------
; Global Messages
;--------------------------------------------------------
Message:    db "We have an error in boot process"
MessageLen: equ $-Message
MsrLoading: db 0x0D, 0x0A, "Loading Boot Files", 0xD, 0x0A, 0x00
MsrJumpStage2: db "Jumping To Location 0x7E00", 0x0A
MsrBootSector: db "Loaded Into Memory Location 0x7C00", 0x0D
;--------------------------------------------------------
; Global Variables
;--------------------------------------------------------
DriveId:    db 0
ReadPacket: times 16 db 0

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