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
    popa

    mov ax, [bp + 12]
    shl eax, 16 ; shift into upper 32 bits
    mov ax, [bp + 10]

    ; add a base sector into start-sector
    add eax, [dBaseSector]

    xor edx, edx
    xor ecx, ecx
    mov cx, WORD[wSectorsPerTrack]
    div ecx
    inc dx
    push dx

    xor dx, dx
    div WORD [wHeadsPerCylinder]
    push dx
    push ax

    mov ax, [bp + 8]
    mov cx, [bp + 6]
    mov dx, [bp + 4]

    push ax
    push cx
    push dx
    STACK_FRAME_END16


Search_Root_Directory_Loop:
    xchg bx, bx
    mov cx, [wRootEntries]
    mov di, [dRootDirStart]
    cmp cx, 0 ; check if we have reached  the end of the root directory
    je File_Not_Found 

    ; Check the root directory for a file
    mov al, BYTE[di + 0x0b]
    and al, 0x18
    cmp al, 0 ; check if the bits 4 and 3 are 0
    jne Not_A_File

    ; Check if the file name we are searching for is the correct one
    mov si, DefStage2
    mov cx, 11 ; File name is 11 bytes long
    mov di, di + 0x0 ; We move the pointer to the beginnning of the file name
    repe cmpsb ; Compare the filename in the root directory with the filename we are looking for
    jz File_Is_Found

    Not_A_File:
    add di, 0x20 ; Move DI to the next directory entry
    dec cx
    jmp Search_Root_Directory_Loop
    
    File_Is_Found:
    ; Let's load this dumbass
    mov ax, word [di + 0x1a]
    jmp Load_File

    File_Not_Found:
    mov si, File_Not_Exist
    call PRINT16BIT

Load_File:
    mov si, FileFoundSuccesfully
    call PRINT16BIT
; *************************
; Global Variables
; *************************
DefStage2	db 	"STAGE2  SYS"
Stage1_JMP_Message db "Jumping to 0x7E00"
DRootLBA dd 0
FileFoundSuccesfully db "That is surreal!!!!"
dBaseSector db 1
; *************************
; Error Codes
; *************************
File_Not_Exist db "File Doesn't Exist In The Root Directory"

; *************************
; Fill Out The Bootloader
; *************************
times 510-($-$$) db 0

; *************************
; Boot Signature
; *************************
db 0x55, 0xAA



; calculate RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec - 1) / BPB_BytsPerSec), It is always 0
    ; RootEntCnt is always 0 too
    ; clear out ebx and eax, lets save the result here to ebx and we can always pop them
    ;xor ebx, ebx
    ;xor eax, eax
    ;mov eax, dword [wRootEntries]
    ;imul eax, 32
    ;mov ecx, dword [wBytesPerSector]
    ;sub ecx, 1
    ;add eax, ecx
    ;xor ecx, ecx
    ;mov ecx, dword [wBytesPerSector]
    ;mov ebx, eax
    ;div ebx, ecx
    ;; save the result
    ;push ebx

FirstSectorofCluster:
    ; FirstSectorofCluster = ((N – 2) * BPB_SecPerClus) + FirstDataSector;
    xor ecx, ecx
    xor eax, eax
    lea edi, [esi-2]
    movzx eax, byte [BPB_SecPerClus]
    imul edi, eax
    add edi, eax
    ; save result in edi
    push edi

DataSec:
    ; DataSec = TotSec – (BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors) ; RootDirSectors
    xor eax, eax
    xor edx, edx
    movzx eax, byte [BPB_NumFATs]
    movzx edx, byte [BPB_FATSz32]
    imul eax, edx
    movzx edx, word [BPB_RsvdSecCnt]
    add eax, edx
    movzx edx, word [BPB_TotSec32]
    sub eax, edx
    push eax

CountofCluster:
    ; CountofClusters = DataSec / BPB_SecPerClus
    pop eax
    mov eax, eax
    movzx eax, byte [BPB_SecPerClus]
    div eax

FirstSectorofCluster2:
    xor ebx, ebx
    xor eax, eax
    
    lea edi, [esi-2]
    movzx eax, byte [BPB_SecPerClus]
    imul edi, eax
    
    pop ebx ; pop the stack value into register ebx from eax
    add edi, ebx
    
    ; save result in edi
    push edi
    mov cx, [BPB_SecPerClus] ; read Cluster


Stage1:
    mov dl, byte [BS_DrvNum] ; DL = Drive Number
    
    ; Lets calculate the FirstDataSector = BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors(Its zero) + Partitiontable
    mov di, PartitionTableData
    mov cx, 0x0008 ; 8 words = 16 bytes
    repnz movsw

    ; clear eax and ebx
    xor eax, eax
    xor ebx, ebx
    
    movzx eax, byte [BPB_NumFATs] ; (BPB_NumFATs * FATSz)
    mov ebx, dword [BPB_FATSz32]
    imul eax, ebx
    
    xor ebx, ebx ; clear it from previous value
    mov ebx, dword [BPB_RsvdSecCnt] ; BPB_ResvdSecCnt + (BPB_NumFATs * FATSz)
    add eax, ebx

    xor ebx, ebx
    mov ebx, dword [dBaseSector]
    add eax, ebx ; BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + Partitiontable

    ; save result
    push eax
    pop eax

find_File:
    xchg bx, bx
    mov dx, word [BPB_RootClus]
    push di

search_Root_Files:
    push di
    mov si, FileName
    mov cx, 11
    rep cmpsb
    pop di
    je read_File

    add di, 32
    dec dx

    call ColdReboot
        

read_File:
    push di
    push es
    pop ds
    pop si

    mov di, 0x7E00
    mov es, di

    pop ax
    call FirstSectorofCluster

    jmp 0x0:0x7E00

FirstSectorofCluster:
    ; FirstSectorofCluster = ((N – 2) * BPB_SecPerClus) + FirstDataSector;
    ; preserve registers
    pushad
    
    lea   edi, [esi-2]
    movzx   eax, byte [BPB_SecPerClus]
    imul  edi, eax

    movzx eax, byte [BPB_NumFATs]
    imul  eax, [BPB_FATSz32]
    add   edi, eax

    movzx eax, word [BPB_RsvdSecCnt]
    add   eax, edi
    add   eax, [BPB_HiddSec]
        
    push  dx
    mov   cx, [BPB_SecPerClus]

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
;   TODO
; 1) Fix the stack mistakes
; 2) Properly read file
; 3) Test
;*************************************************;



;*************************************************;
;   Global Variables
;*************************************************;
PartitionTableData dq 0
dBaseSector       dd 0

dSectorCount      dd 0

FileName   db "STAGE2  SYS"
;DAP - Disk Address Packet, Used by INT13h Extensions	
disk_Address_Packet:
    DAP_Size					db 0x10			;Size of packet, always 16 BYTES
    DAP_Reserved				db 0x00			;Reserve; 16 Bit Code, Origin at 0x0
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
    popa

    mov ax, [bp + 12]
    shl eax, 16 ; shift into upper 32 bits
    mov ax, [bp + 10]

    ; add a base sector into start-sector
    add eax, [dBaseSector]

    xor edx, edx
    xor ecx, ecx
    mov cx, WORD[wSectorsPerTrack]
    div ecx
    inc dx
    push dx

    xor dx, dx
    div WORD [wHeadsPerCylinder]
    push dx
    push ax

    mov ax, [bp + 8]
    mov cx, [bp + 6]
    mov dx, [bp + 4]

    push ax
    push cx
    push dx
    STACK_FRAME_END16


Search_Root_Directory_Loop:
    xchg bx, bx
    mov cx, [wRootEntries]
    mov di, [dRootDirStart]
    cmp cx, 0 ; check if we have reached  the end of the root directory
    je File_Not_Found 

    ; Check the root directory for a file
    mov al, BYTE[di + 0x0b]
    and al, 0x18
    cmp al, 0 ; check if the bits 4 and 3 are 0
    jne Not_A_File

    ; Check if the file name we are searching for is the correct one
    mov si, DefStage2
    mov cx, 11 ; File name is 11 bytes long
    mov di, di + 0x0 ; We move the pointer to the beginnning of the file name
    repe cmpsb ; Compare the filename in the root directory with the filename we are looking for
    jz File_Is_Found

    Not_A_File:
    add di, 0x20 ; Move DI to the next directory entry
    dec cx
    jmp Search_Root_Directory_Loop
    
    File_Is_Found:
    ; Let's load this dumbass
    mov ax, word [di + 0x1a]
    jmp Load_File

    File_Not_Found:
    mov si, File_Not_Exist
    call PRINT16BIT

Load_File:
    mov si, FileFoundSuccesfully
    call PRINT16BIT
; *************************
; Global Variables
; *************************
DefStage2	db 	"STAGE2  SYS"
Stage1_JMP_Message db "Jumping to 0x7E00"
DRootLBA dd 0
FileFoundSuccesfully db "That is surreal!!!!"
dBaseSector db 1
; *************************
; Error Codes
; *************************
File_Not_Exist db "File Doesn't Exist In The Root Directory"

; *************************
; Fill Out The Bootloader
; *************************
times 510-($-$$) db 0

; *************************; 16 Bit Code, Origin at 0x0
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
    popa

    mov ax, [bp + 12]
    shl eax, 16 ; shift into upper 32 bits
    mov ax, [bp + 10]

    ; add a base sector into start-sector
    add eax, [dBaseSector]

    xor edx, edx
    xor ecx, ecx
    mov cx, WORD[wSectorsPerTrack]
    div ecx
    inc dx
    push dx

    xor dx, dx
    div WORD [wHeadsPerCylinder]
    push dx
    push ax

    mov ax, [bp + 8]
    mov cx, [bp + 6]
    mov dx, [bp + 4]

    push ax
    push cx
    push dx
    STACK_FRAME_END16


Search_Root_Directory_Loop:
    xchg bx, bx
    mov cx, [wRootEntries]
    mov di, [dRootDirStart]
    cmp cx, 0 ; check if we have reached  the end of the root directory
    je File_Not_Found 

    ; Check the root directory for a file
    mov al, BYTE[di + 0x0b]
    and al, 0x18
    cmp al, 0 ; check if the bits 4 and 3 are 0
    jne Not_A_File

    ; Check if the file name we are searching for is the correct one
    mov si, DefStage2
    mov cx, 11 ; File name is 11 bytes long
    mov di, di + 0x0 ; We move the pointer to the beginnning of the file name
    repe cmpsb ; Compare the filename in the root directory with the filename we are looking for
    jz File_Is_Found

    Not_A_File:
    add di, 0x20 ; Move DI to the next directory entry
    dec cx
    jmp Search_Root_Directory_Loop
    
    File_Is_Found:
    ; Let's load this dumbass
    mov ax, word [di + 0x1a]
    jmp Load_File

    File_Not_Found:
    mov si, File_Not_Exist
    call PRINT16BIT

Load_File:
    mov si, FileFoundSuccesfully
    call PRINT16BIT
; *************************
; Global Variables
; *************************
DefStage2	db 	"STAGE2  SYS"
Stage1_JMP_Message db "Jumping to 0x7E00"
DRootLBA dd 0
FileFoundSuccesfully db "That is surreal!!!!"
dBaseSector db 1
; *************************
; Error Codes
; *************************
File_Not_Exist db "File Doesn't Exist In The Root Directory"

; *************************
; Fill Out The Bootloader
; *************************
times 510-($-$$) db 0

; *************************
; Boot Signature
; *************************; 16 Bit Code, Origin at 0x0
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
    popa

    mov ax, [bp + 12]
    shl eax, 16 ; shift into upper 32 bits
    mov ax, [bp + 10]

    ; add a base sector into start-sector
    add eax, [dBaseSector]

    xor edx, edx
    xor ecx, ecx
    mov cx, WORD[wSectorsPerTrack]
    div ecx
    inc dx
    push dx

    xor dx, dx
    div WORD [wHeadsPerCylinder]
    push dx
    push ax

    mov ax, [bp + 8]
    mov cx, [bp + 6]
    mov dx, [bp + 4]

    push ax
    push cx
    push dx
    STACK_FRAME_END16


Search_Root_Directory_Loop:
    xchg bx, bx
    mov cx, [wRootEntries]
    mov di, [dRootDirStart]
    cmp cx, 0 ; check if we have reached  the end of the root directory
    je File_Not_Found 

    ; Check the root directory for a file
    mov al, BYTE[di + 0x0b]
    and al, 0x18
    cmp al, 0 ; check if the bits 4 and 3 are 0
    jne Not_A_File

    ; Check if the file name we are searching for is the correct one
    mov si, DefStage2
    mov cx, 11 ; File name is 11 bytes long
    mov di, di + 0x0 ; We move the pointer to the beginnning of the file name
    repe cmpsb ; Compare the filename in the root directory with the filename we are looking for
    jz File_Is_Found

    Not_A_File:
    add di, 0x20 ; Move DI to the next directory entry
    dec cx
    jmp Search_Root_Directory_Loop
    
    File_Is_Found:
    ; Let's load this dumbass
    mov ax, word [di + 0x1a]
    jmp Load_File

    File_Not_Found:
    mov si, File_Not_Exist
    call PRINT16BIT

Load_File:
    mov si, FileFoundSuccesfully
    call PRINT16BIT
; *************************
; Global Variables
; *************************
DefStage2	db 	"STAGE2  SYS"
Stage1_JMP_Message db "Jumping to 0x7E00"
DRootLBA dd 0
FileFoundSuccesfully db "That is surreal!!!!"
dBaseSector db 1
; *************************
; Error Codes
; *************************
File_Not_Exist db "File Doesn't Exist In The Root Directory"

; *************************
; Fill Out The Bootloader
; *************************
times 510-($-$$) db 0

; *************************
; Boot Signature
; *************************
db 0x55, 0xAA

; Calling convention helpers
%macro STACK_FRAME_BEGIN 0
    push bp
    mov bp, sp
%endmacro
%macro STACK_FRAME_END 0
    mov sp, bp
    pop bp
%endmacro

ImageLoadSeg EQU 60h     ; <=07Fh because of ; 16 Bit Code, Origin at 0x0
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
    popa

    mov ax, [bp + 12]
    shl eax, 16 ; shift into upper 32 bits
    mov ax, [bp + 10]

    ; add a base sector into start-sector
    add eax, [dBaseSector]

    xor edx, edx
    xor ecx, ecx
    mov cx, WORD[wSectorsPerTrack]
    div ecx
    inc dx
    push dx

    xor dx, dx
    div WORD [wHeadsPerCylinder]
    push dx
    push ax

    mov ax, [bp + 8]
    mov cx, [bp + 6]
    mov dx, [bp + 4]

    push ax
    push cx
    push dx
    STACK_FRAME_END16


Search_Root_Directory_Loop:
    xchg bx, bx
    mov cx, [wRootEntries]
    mov di, [dRootDirStart]
    cmp cx, 0 ; check if we have reached  the end of the root directory
    je File_Not_Found 

    ; Check the root directory for a file
    mov al, BYTE[di + 0x0b]
    and al, 0x18
    cmp al, 0 ; check if the bits 4 and 3 are 0
    jne Not_A_File

    ; Check if the file name we are searching for is the correct one
    mov si, DefStage2
    mov cx, 11 ; File name is 11 bytes long
    mov di, di + 0x0 ; We move the pointer to the beginnning of the file name
    repe cmpsb ; Compare the filename in the root directory with the filename we are looking for
    jz File_Is_Found

    Not_A_File:
    add di, 0x20 ; Move DI to the next directory entry
    dec cx
    jmp Search_Root_Directory_Loop
    
    File_Is_Found:
    ; Let's load this dumbass
    mov ax, word [di + 0x1a]
    jmp Load_File

    File_Not_Found:
    mov si, File_Not_Exist
    call PRINT16BIT

Load_File:
    mov si, FileFoundSuccesfully
    call PRINT16BIT
; *************************
; Global Variables
; *************************
DefStage2	db 	"STAGE2  SYS"
Stage1_JMP_Message db "Jumping to 0x7E00"
DRootLBA dd 0
FileFoundSuccesfully db "That is surreal!!!!"
dBaseSector db 1
; *************************
; Error Codes
; *************************
File_Not_Exist db "File Doesn't Exist In The Root Directory"

; *************************
; Fill Out The Bootloader
; *************************
times 510-($-$$) db 0

; *************************
; Boot Signature
; *************************
db 0x55, 0xAA
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
    cli                        ; Disable Interrupts
    
    mov ax, 0x0000             ; Set ax to 0

    mov ds, ax                 ; Set segment ds to 0
    
    mov es, ax                 ; Set segment es to 0
    
    mov ss, ax                 ; Set segment ss to 0
    
    mov sp, 0x7C00             ; Set stack-pointer to 0x7C00

checkStack:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;; How much RAM is there? ;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    int 12h                    ; Get size in KB
    
    shl ax, 6                  ; convert it to 16-byte paragraphs

    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;; Reserve memory for the boot sector and its stack ;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    sub ax, 512/16             ; reserve 512 bytes for the bootsector

    mov es, ax                 ; es:0 -> top - 512

    sub     ax, 2048 / 16      ; reserve 2048 bytes for the stack
    
    mov     ss, ax             ; ss:0 -> top - 512 - 2048
    
    mov     sp, 2048           ; 2048 bytes for the stack

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;; Copy ourselves to top of memory ;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov     cx, 256

    mov     si, 7C00h
    
    xor     di, di
    
    mov     ds, di
    
    rep     movsw

    ;;;;;;;;;;;;;;;;;;;;;;
    ;; Jump to the copy ;;
    ;;;;;;;;;;;;;;;;;;;;;;

    push    es
    
    push    byte Stage1
    

Stage1:
    xchg bx, bx
    push cs

    pop ds
    
    ; Lets calculate the FirstDataSector = BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors(Its zero) + Partitiontable
    mov di, PartitionTableData
    mov cx, 0x0008 ; 8 words = 16 bytes
    repnz movsw


    mov byte [BS_DrvNum], dl

    and     byte [BPB_RootClus+3], 0Fh ; mask cluster value
    
    mov     esi, [BPB_RootClus] ; esi=cluster # of root dir

RootDirReadContinue:
        push    byte ImageLoadSeg
        pop     es
        xor     bx, bx
        call    ReadCluster             ; read one cluster of root dir
        push    esi                     ; save esi=next cluster # of root dir
        pushf                           ; save carry="not last cluster" flag

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Look for the COM/EXE file to load and run ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        push    byte ImageLoadSeg
        pop     es
        xor     di, di                  ; es:di -> root entries array
        mov     si, FileName         ; ds:si -> program name

        mov     al, [BPB_SecPerClus]
        cbw
        mul     word [BPB_BytsPerSec]; ax = bytes per cluster
        shr     ax, 5
        mov     dx, ax                  ; dx = # of dir entries to search in

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Looks for a file/dir by its name      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Input:  DS:SI -> file name (11 chars) ;;
;;         ES:DI -> root directory array ;;
;;         DX = number of root entries   ;;
;; Output: ESI = cluster number          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

FindName:
        mov     cx, 11
FindNameCycle:
        cmp     byte [es:di], ch
        jne     FindNameNotEnd
        jmp     ErrFind                 ; end of root directory (NULL entry found)
FindNameNotEnd:
        pusha
        repe    cmpsb
        popa
        je      FindNameFound
        add     di, 32
        dec     dx
        jnz     FindNameCycle           ; next root entry
        popf                            ; restore carry="not last cluster" flag
        pop     esi                     ; restore esi=next cluster # of root dir
        jc      RootDirReadContinue     ; continue to the next root dir cluster
        jmp     ErrFind                 ; end of root directory (dir end reached)
FindNameFound:
        push    word [es:di+14h]
        push    word [es:di+1Ah]
        pop     esi                     ; si = cluster no.

;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Load the entire file ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;

        push    byte ImageLoadSeg
        pop     es
        xor     bx, bx

ReadCluster:
        mov     ax, [BPB_BytsPerSec]
        shr     ax, 2                           ; ax=# of FAT32 entries per sector
        cwde
        mov     ebp, esi                        ; ebp=esi=cluster #
        xchg    eax, esi
        cdq
        div     esi                             ; eax=FAT sector #, edx=entry # in sector
        movzx   edi, word [BPB_RsvdSecCnt]
        add     edi, [BPB_HiddSec]
        add     eax, edi

        push    dx                              ; save dx=entry # in sector on stack
        mov     cx, 1
        call    ReadSectorLBA                   ; read 1 FAT32 sector

        pop     si                              ; si=entry # in sector
        add     si, si
        add     si, si
        and     byte [es:si+3], 0Fh             ; mask cluster value
        mov     esi, [es:si]                    ; esi=next cluster #

        lea     eax, [ebp-2]
        movzx   ecx, byte [BPB_SecPerClus]
        mul     ecx
        mov     ebp, eax

        movzx   eax, byte [BPB_NumFATs]
        mul     dword [BPB_FATSz32]

        add     eax, ebp
        add     eax, edi

        call    ReadSectorLBA

        mov     ax, [BPB_BytsPerSec]
        shr     ax, 4                   ; ax = paragraphs per sector
        mul     cx                      ; ax = paragraphs read

        mov     cx, es
        add     cx, ax
        mov     es, cx                  ; es:bx updated

        cmp     esi, 0FFFFFF8h          ; carry=0 if last cluster, and carry=1 otherwise
        ret

ReadSectorLBA:
        pushad

ReadSectorLBANext:
        pusha

        push    byte 0
        push    byte 0 ; 32-bit LBA only: up to 2TB disks
        push    eax
        push    es
        push    bx
        push    byte 1 ; sector count word = 1
        push    byte 16 ; packet size byte = 16, reserved byte = 0

        mov     ah, 42h
        mov     dl, [BS_DrvNum]
        mov     si, sp
        push    ss
        pop     ds
        int     13h
        push    cs
        pop     ds

        jc      short ErrRead
        add     sp, 16 ; the two instructions are swapped so as not to overwrite carry flag

        popa
        dec     cx
        jz      ReadSectorLBADone2      ; last sector

        add     bx, [BPB_BytsPerSec] ; adjust offset for next sector
        add     eax, byte 1             ; adjust LBA for next sector
        jmp     short ReadSectorLBANext

ReadSectorLBADone2:
        popad
        ret

ErrRead:
ErrFind:
    mov si, FindFileNameError
    call PRINT16BIT
    hlt
;*****************************************************************************************************************;
;                          DATA   SECTION                                                                         ;
;*****************************************************************************************************************;
PartitionTableData      dq 0
dBaseSector             dd 0
dSectorCount            dd 0
FileName                db "STAGE2  SYS"

;*************************************************;
;             Stage1 Messages                     ;
;*************************************************;



;*************************************************;
;             Error Handlers                      ;
;*************************************************;
FindFileNameError       db "Can't find file"

times 510-($-$$) DB 0
dw 0xAA55