; *************************
;    Real Mode 16-Bit
; - Uses the native Segment:offset memory model
; - Limited to 1MB of memory
; - No memory protection or virtual memory
; *************************
BITS	16							; We are in 16 bit Real Mode

ORG		0x7C00						; We are loaded by BIOS at 0x7C00

MAIN:       JMP STAGE1              ; Jump over the Fat32 Blocks

; *************************
; FAT Boot Parameter Block
; *************************
szOemName					DB		"MY    OS"
wBytesPerSector				DW		0
bSectorsPerCluster			DB		0
wReservedSectors			DW		0
bNumFATs					DB		0
wRootEntries				DW		0
wTotalSectors				DW		0
bMediaType					DB		0
wSectorsPerFat				DW		0
wSectorsPerTrack			DW		0
wHeadsPerCylinder			DW		0
dHiDDenSectors				DD 		0
dTotalSectors				DD 		0

; *************************
; FAT32 Extension Block
; *************************
dSectorsPerFat32			DD 		0
wFlags						DW		0
wVersion					DW		0
dRootDirStart				DD 		0
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
			LODSB					        ; load next byte from string from SI to AL

			OR			al, al		        ; Does AL=0?
			
            JZ			PRINTDONE16BIT	    ; Yep, null terminator found-bail out
			
            MOV			ah,	0eh	            ; Nope-Print the character
			
            INT			10h
			
            JMP			PRINT16BIT		    ; Repeat until null terminator found

PRINTDONE16BIT:
			
            RET					            ; we are done, so return

;*************************************************;
;	Bootloader Entry Point
;*************************************************;

STAGE1:
    ;----------------------------------------------------
    ; code located at 0000:7C00, adjust segment registers
    ;----------------------------------------------------
    
    CLI                                    ; disable interrupts
    
    XOR ax,ax   
    
    MOV ds,ax
    
    MOV es,ax  
    
    MOV ss,ax
    
    ;----------------------------------------------------
    ; create stack
    ;----------------------------------------------------
    
    MOV sp,0x7c00
    
    MOV         si, msgStart
    
    CALL        PRINT16BIT                     ; Error message
    
    STI                                    ; restore interrupts


TESTDISKEXTENTION:
    
    MOV [bPhysicalDriveNum],dl
    
    MOV ah,0x41
    
    MOV bx,0x55aa
    
    INT 0x13
    
    JC DEATHSCREEN
    
    CMP bx,0xaa55
    
    JNE DEATHSCREEN

STAGE2LOADER:
    ; DS:SI (segment:offset pointer to the DAP, Disk Address Packet)
    ; DS is Data Segment, SI is Source Index
    MOV         si,ReadPacket
    ; size of Disk Address Packet (set this to 0x10)
    MOV         word[si],0x10
    ; number of sectors(loader) to read
    MOV         word[si+2],5
    ; number of sectors to transfer
    ; transfer buffer (16 bit segment:16 bit offset)
    ; 16 bit offset=0x7e00 (stored in word[si+4])
    ; 16 bit segment=0 (stored in word[si+6])
    ; address => 0 * 16 + 0x7e00 = 0x7e00
    MOV         word[si+4],0x7e00
    
    MOV         word[si+6],0
    ; absolute number of the start of the sectors to be read
    ; LBA=1 (the 2nd sector) is the start of sector to be read
    ; LBA=1 is start of loader sector
    ; lower part of 64-bit starting LBA
    MOV         DWord[si+8],1
    ; upper part of 64-bit starting LBA
    MOV         DWord[si+0xc],0
    ; dl=drive id
    MOV         dl,[bPhysicalDriveNum]
    ; function code, 0x42 = Extended Read Sectors From Drive
    MOV         ah,0x42
    
    INT         0x13
    
    JC          DEATH

    MOV         dl,[bPhysicalDriveNum]

    MOV         si, msgStageTwo
    
    CALL        PRINT16BIT                     ; Error message
    
    JMP         0x7e00

DEATH:
DEATHSCREEN:
    
    MOV         si, msg
    
    CALL        PRINT16BIT                     ; Error message
    
    MOV         ah, 0x00
    
    INT         0x16                           ; SMACK YOUR ASS ON THAT KEYBOARD AGAIN
    
    INT         0x19                           ; Reboot and try again, bios uses int 0x19 to find a bootable device

END:
    HLT
    
    JMP END

;*******************************************************
;	Data Section
;*******************************************************
msg	                        DB	"Hahaha, Fuck you, but at stage1", 0
MessageLenmsg:              equ $-msg
ReadPacket:                 TIMES 16 DB 0
msgStart                    DB 0x0D,0x0A, "Boot Loader starting (0x7C00)....", 0x0D, 0x0A, 0x00
msgStageTwo                 DB "Jumping to stage2 (0x7E00)", 0x0D, 0x0A, 0x0D, 0x0A, 0x00

; 0x1be = 446 bytes, which is bootloader code size
; $-$$ is current section size (in bytes)
TIMES (0x1BE-($-$$)) DB 0
    ; here are partition table entries (which is total 64 bytes)
    ; there are 4 entries and only define the first entry
    ; each entry has 16 bytes (64/4=16)
    ; this is the first partition entry format
    ; boot indicator, 0x80=bootable partition
    DB 80H
    ; starting CHS
    ; C=cylinder, H=head, S=sector
    ; the range for cylinder is 0 through 1023
    ; the range for head is 0 through 255 inclusive
    ; The range for sector is 1 through 63
    ; address of the first sector in partition
    ; 0(head),2(sector),0(cylinder)
    DB 0,2,0
    ; partition type
    DB 0F0H
    ; ending of CHS
    ; address of last absolute sector in partition
    DB 0FFH,0FFH,0FFH
    ; LBA of the first absolute sector
    ; LBA = (C × HPC + H) × SPT + (S − 1)
    ; S(sector)=2, C(cylinder)=0, H(head)=0 => LBA=1
    DD 1
    ; size (number of sectors in partition)
    ; here is (20*16*63-1) * 512 bytes/ (1024 * 1024) = 10 mb
    DD (20*16*63-1)
	; other 3 entries are set to 0
    TIMES (16*3) DB 0
    ; Boot Record signature
    ; here is the same as dw 0aa55h
    ; little endian
    DW 0xAA55                              ; fill out the last 2 bytes of our stage1 bootloader