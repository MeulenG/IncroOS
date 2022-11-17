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
BS_OEMName					db		"MSWIN4.1"
BPB_BytsPerSec				dw		0
BPB_SecPerClus  			db		0
BPB_RsvdSecCnt  			dw		0
BPB_NumFATs					db		0
BPB_RootEntCnt				dw		0
BPB_TotSec16				dw		0
BPB_Media					db		0
BPB_FatSz16 				dw		0
BPB_SecPerTrk   			dw		0
BPB_NumHeads    			dw		0
BPB_HiddSec 				dd 		0
BPB_TotSec32				dd 		0

; *************************
; FAT32 Extension Block
; *************************
BPB_FATSz32     			dd 		0
BPB_ExtFlags				dw		0
BPB_FSVer					dw		0
BPB_RootClus				dd 		0
BPB_FSInfo  				dw		0
BPB_BkBootSec   			dw		0

; Reserved 
dReserved0					dd		0 	;FirstDataSector

BS_DrvNum       			db		0

dReserved1					dd		0 	;ReadCluster

BS_BootSig  				db		0
dReserved2					dd 		0 	;ReadCluster

bReserved3					db		0

BS_VolID    				dd 		0

BS_VolLab   				db		"NO NAME    "

BS_FilSysType				db		"FAT32   "

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
    cli ; Disable Interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    push ax

; Read the disk using INT13h
read_Disk:
    pushad ; preserve registers
    mov dl, byte [BS_DrvNum] ; DL = Drive Number
    mov ah, 0x42 ; Extended Read Sectors from Drive
    lea si, [DAP_Size] ; DI:SI = Offset of DAP
    int 0x13 ; Read Disk
    popa ; restore registers
    ret ; return
;---------------------------------------------------------------------------------------------------------------------------------;

Dir_Entry_Struct:
DE_Short_Name times 11 		db 0x00			;8.3(short) File Name
DE_Attrib					db 0x00			;File attributes
DE_Reserved					db 0x00			;Reserved by NT
DE_Create_Time_Tenth		db 0x00			;Creation time in 10ths of a second
DE_Create_Time				dw 0x0000		;Creation time
DE_Create_Date				dw 0x0000		;Creation date
DE_Last_Access_Date			dw 0x0000		;Date that file was last accessed
DE_Cluster_MSW				dw 0x0000		;MSW of first cluster in chain
DE_Last_Mod_Time			dw 0x0000		;Last modification time
DE_Last_Mod_Date			dw 0x0000		;Last modification date
DE_Cluster_LSW				dw 0x0000		;LSW of first cluster in chain
DE_Size						dd 0x00000000	;Size of file in bytes
;Finally there is the start cluster for the current directory, it is set to the root directory upon initilization.
Current_Dir_Cluster			dd 0x00000000	;Used to change directories
;---------------------------------------------------------------------------------------------------------------------------------;
Stage2:
    ; Lets calculate the FirstDataSector = BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors + Partitiontable
    mov di, PartitionTableData
    mov cx, 0x0008 ; 8 words = 16 bytes
    repnz movsw
    ; clear ax
    xor edx, edx
    movzx eax, byte [BPB_NumFATs]
    ;movzx ebx, dword [BPB_FATSz32]
    mul bx
    ;mov ax, dword [BPB_RsvdSecCnt]
    add ax, bx
    ; save result
    push ax

    mov eax, dword [BPB_RootClus] 
    lea si, [FileName]
    mov bx, STAGE2_SYS_ADDRESS
    call read_File
    ;jc STAGE2_SYS_MISSING
    push cs
    push STAGE2_SYS_ADDRESS
    retf ; let our stage 2 take over hopefully lol

read_File:
    pushad ; preserve registers
    ;call read_Directory
    jc read_File_Return
    mov ax, word [DE_Cluster_MSW]
    shl eax, 16
    mov ax, word [DE_Cluster_LSW]
    ;call read_Chain

read_File_Return:
    popad ;restore registers
    ret

Disk_Error:
    mov si, Disk_Error
    call PRINT16BIT

ColdReboot:
    mov si, HALT_MSG
    call PRINT16BIT
    cli
    hlt

;*************************************************;
;   Global Variables
;*************************************************;
PartitionTableData dq 0
FileName   db "STAGE2  SYS"
FAT_BUFFER  				EQU 0x8600		;Pointer to buffer to hold FAT sectors ; hvor satan starter den henne and how to see
DIR_BUFFER					EQU 0x8800		;Pointer to buffer to hold sectors of Directory ; hvor satan starter den henne
STAGE2_SYS_ADDRESS			EQU 0x4B8D		;Base address and entry point for Stage3
;DAP - Disk Address Packet, Used by INT13h Extensions	
DAP_Size					db 0x10			;Size of packet, always 16 BYTES
DAP_Reserved				db 0x00			;Reserved
DAP_Sectors					dw 0x0000		;Number of sectors to read
DAP_Offset					dw 0x0000		;Offset to read data to
DAP_Segment 				dw 0x0000		;Segment to read data to
DAP_LBA_LSD					dd 0x00000000	;QWORD with LBA of where to start reading from
DAP_LBA_MSD					dd 0x00000000
;*************************************************;
;   Error Messages
;*************************************************;
DISK_ERROR	db "Error reading from disk!",0
HALT_MSG	db 0x0D,0x0A,"Halting machine. Power off and reboot.",0


;*************************************************;
;   Data
;*************************************************;
loadFileError db "Can't load file"
findFileError db "Unable to find your file"

times 510-($-$$) DB 0
dw 0xAA55