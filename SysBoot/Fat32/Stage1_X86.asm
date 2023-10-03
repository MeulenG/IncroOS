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
szOemName					db		"Puhaa-OS"
wBytesPerSector				dw		0 ; 0B
bSectorsPerCluster			db		0 ; 0D
wReservedSectors			dw		0 ; 0E
bNumFATs					db		0 ; 10
wRootEntries				dw		0 ; 11
wTotalSectors				dw		0 ; 13
bMediaType					db		0 ; 15
wSectorsPerFat				dw		0 ; 16
wSectorsPerTrack			dw		0 ; 18
wHeadsPerCylinder			dw		0 ; 1A
dHiddenSectors				dd 		0 ; 1C
dTotalSectors				dd 		0 ; 20

; *************************
; FAT32 Extension Block
; *************************
dSectorsPerFat32			dd 		0 ; 24
wFlags						dw		0 ; 28
wVersion					dw		0 ; 2A
dRootDirStart				dd 		0 ; 2C
wFSInfoSector				dw		0 ; 30
wBackupBootSector			dw		0 ; 32

; Reserved
dReserved0					dd		0 	; FirstDataSector ; 34
dReserved1					dd		0 	; ReadCluster     ; 38
dReserved2					dd 		0 	; ReadCluster     ; 3C

bPhysicalDriveNum			db		0 ; 40
bReserved3					db		0
bBootSignature				db		0
dVolumeSerial				dd 		0
szVolumeLabel				db		"NO NAME    "
szFSName					db		"FAT32   "

%include "../Routines/stdio16.inc"
Main:
    ;-------------------------------------------------------
    ; Let's set the stack first thing
    ;-------------------------------------------------------
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

TestDiskExtension:
    mov [DriveId],dl

    mov ah,0x41

    mov bx,0x55aa

    int 0x13

    jc NotSupport

    cmp bx,0xaa55

    jne NotSupport

LoadStage2:
    ; Copy FAT table to a fixed address
    mov si, FATTableBuffer
    mov ax, 0x6000
    mov es, ax
    xor di, di
    mov cx, 79
    rep movsb
    ; DS:SI (segment:offset pointer to the DAP, Disk Address Packet)
    ; DS is Data Segment, SI is Source Index
    mov si,ReadPacket
    ; size of Disk Address Packet (set this to 0x10)
    mov word[si],0x10
    ; number of sectors(loader) to read(in this case its 100 sectors for our kernel)
    mov word[si+2],5
    ; number of sectors to transfer
    ; transfer buffer (16 bit segment:16 bit offset)
    ; 16 bit offset=0 (stored in word[si+4])
    mov word[si+4],0x7e00
    ; 16 bit segment=0x1000 (stored in word[si+6])
    ; address => 0 * 16 + 0xA000 = 0xA000
    mov word[si+6],0
    ; LBA=6 is the start of kernel sector
    mov dword[si+8],1

    mov dword[si+0xc],0
    ; dl=pysicaldrivenum
    mov dl,[DriveId]
    ; function code, 0x42 = Extended Read Sectors From Drive
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
FATTableBuffer: times 79 db 0

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