; *******************************************************
; Memory Map:
; 0x00000000 - 0x000004FF		Reserved
; 0x00000500 - 0x00007AFF		Second Stage Bootloader (~29 Kb)
; 0x00007B00 - 0x00007BFF		Stack Space (256 Bytes)
; 0x00007C00 - 0x00007DFF		Bootloader (512 Bytes)
; 0x00007E00 - 0x0007FFFF		Kernel Loading Bay (480.5 Kb)
; Rest above is not reliable

; In this bootloader we use the kernel loading bay a lot
; 0x00007E00 - 0x00007FFF 		GetNextCluster (Usage)
; 0x00008000 - ClusterSize		Used by LocateFile Procedure

; 16 Bit Code, Origin at 0x0
BITS 16
ORG 0x7C00


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

; *************************
; Contains Print Routine
; *************************
%include "../includes/stdio16.inc"
; *************************
; Bootloader Entry Point
; *************************

Main:
	; Disable Interrupts, unsafe passage
	cli

	; Far jump to fix segment registers
	jmp 	0x0:FixCS

FixCS:
    ; Fix segment registers to 0
	xor 	ax, ax
	mov		ds, ax
	mov		es, ax

	; Set stack
	mov		ss, ax
	mov		ax, 0x7C00
	mov		sp, ax

	; Done, now we need interrupts again
	sti

	; Step 0. Save DL
	mov 	byte [bPhysicalDriveNum], dl

	; Step 1. Calculate FAT32 Data Sector
	xor		eax, eax
	mov 	al, [bNumFATs]
	mov 	ebx, dword [dSectorsPerFat32]
	mul 	ebx
	xor 	ebx, ebx
	mov 	bx, word [wReservedSectors]
	add 	eax, ebx
	mov 	dword [dReserved0], eax

	; Step 2. Read FAT Table
	mov 	esi, dword [dRootDirStart]

	; Read Loop
	.cLoop:
		mov 	bx, 0x0000
		mov 	es, bx
		mov 	bx, 0x1000
		
		; ReadCluster returns next cluster in chain
		call 	ReadCluster
		push 	esi

		; Step 3. Parse entries and look for Kernel
		mov 	di, 0x1000
		mov 	si, szStage2
		mov 	cx, 0x000B
		mov 	dx, 0x0020
		;mul by bSectorsPerCluster

		; End of root?
		.EntryLoop:
			cmp 	[es:di], ch
			je 		.cEnd

			; No, phew, lets check if filename matches
			cld
			pusha
        	repe    cmpsb
        	popa
        	jne 	.Next

        	; YAY WE FOUND IT!
        	; Get clusterLo & clusterHi
        	push    word [es:di + 14h]
        	push    word [es:di + 1Ah]
        	pop     esi
        	pop 	eax ; fix stack
        	call 	LoadFile
			ret

        	; Next entry
        	.Next:
        		add     di, 0x20
        		dec 	dx
        		jnz 	.EntryLoop

		; Dont loop if esi is above 0x0FFFFFFF5
		pop 	esi
		cmp 	esi, 0x0FFFFFF8
		jb 		.cLoop

	; Ehh if we reach here, not found :s
	.cEnd:
    mov si, ErrorFixCS
    call Puts16
	cli
	hlt

; **************************
; Load 2 Stage Bootloader
; IN:
; 	- ESI Start cluster of file
; **************************
LoadFile:
	push di
	; Lets load the fuck out of this file
	; Step 1. Setup buffer
	mov 	bx, 0x0000
	mov 	es, bx
	mov 	bx, 0x7E00

	; Load
	.cLoop:
		; Clustertime
		call 	ReadCluster

		; Check
		cmp 	esi, 0x0FFFFFF8
		jb 		.cLoop

	; Done, jump
	mov 	dl, byte [bPhysicalDriveNum]
	mov 	dh, 4
	jmp 	0x0:0x7E00

	; Safety catch
	cli
	hlt


; **************************
; FAT ReadCluster
; IN:
;	- ES:BX Buffer
;	- SI ClusterNum
;
; OUT:
;	- ESI NextClusterInChain
; **************************
ReadCluster:
	pusha

	; Save Bx
	push 	bx

	; Calculate Sector
	; FirstSectorofCluster = ((N – 2) * BPB_SecPerClus) + FirstDataSector;
	xor 	eax, eax
	xor 	bx, bx
	xor 	ecx, ecx
	mov 	ax, si
	sub 	ax, 2
	mov 	bl, byte [bSectorsPerCluster]
	mul 	bx
	add 	eax, dword [dReserved0]

	; Eax is now the sector of data
	pop 	bx
	mov 	cl, byte [bSectorsPerCluster]

	; Read
	call 	ReadSector

	; Save position
	mov 	word [dReserved2], bx
	push 	es

	; Si still has cluster num, call next
	call 	GetNextCluster
	mov 	dword [dReserved1], esi

	; Restore
	pop 	es

	; Done
	popa
	mov 	bx, word [dReserved2]
	mov 	esi, dword [dReserved1]
	ret

; **************************
; BIOS ReadSector 
; IN:
; 	- ES:BX: Buffer
;	- AX: Sector start
; 	- CX: Sector count
;
; Registers:
; 	- Conserves all but ES:BX
; **************************
ReadSector:
	; Error Counter
	.Start:
		mov 	di, 5

	.sLoop:
		; Save states
		push 	ax
		push 	bx
		push 	cx

		; Convert LBA to CHS
		xor     dx, dx
        div     WORD [wSectorsPerTrack]
        inc     dl ; adjust for sector 0
        mov     cl, dl ;Absolute Sector
        xor     dx, dx
        div     WORD [wHeadsPerCylinder]
        mov     dh, dl ;Absolute Head
        mov     ch, al ;Absolute Track

        ; Bios Disk Read -> 01 sector
		mov 	ax, 0x0201
		mov 	dl, byte [bPhysicalDriveNum]
		int 	0x13
		jnc 	.Success

	.Fail:
		; HAHA fuck you
		xor 	ax, ax
		int 	0x13
		dec 	di
		pop 	cx
		pop 	bx
		pop 	ax
		jnz 	.sLoop
		
		; Give control to next OS, we failed 
        mov si, ErrorReadSector
        call Puts16
		cli
		hlt

	.Success:
		; Next sector
		pop 	cx
		pop 	bx
		pop 	ax

		add 	bx, word [wBytesPerSector]
		jnc 	.SkipEs
		mov 	dx, es
		add 	dh, 0x10
		mov 	es, dx

	.SkipEs:
		inc 	ax
		loop 	.Start

	; Done
	ret

; **************************
; GetNextCluster
; IN:
; 	- SI ClusterNum
;
; OUT:
;	- ESI NextClusterNum
;
; Registers:
; 	- Trashes EAX, BX, ECX, EDX, ES
; **************************
GetNextCluster:
	; Calculte Sector in FAT
	xor 	eax, eax
	xor 	edx, edx
	mov 	ax, si
	shl 	ax, 2 			; REM * 4, since entries are 32 bits long, and not 8
	div 	word [wBytesPerSector]
	add 	ax, word [wReservedSectors]
	push 	dx

	; AX contains sector
	; DX contains remainder
	mov 	ecx, 1
	mov 	bx, 0x0000
	mov 	es, bx
	mov 	bx, 0x4000
	push 	es
	push 	bx

	; Read Sector
	call 	ReadSector
	pop 	bx
	pop 	es

	; Find Entry
	pop 	dx
	xchg 	si, dx
	mov 	esi, dword [es:bx + si]
	ret
; **************************
; Variables
; **************************
szStage2					db 		"STAGE2  SYS"
ErrorFixCS:  DB "FixCS error", 0x00

ErrorReadSector: DB "ReadSector error", 0x00

; Fill out bootloader
times 510-($-$$) db 0

; Boot Signature
db 0x55, 0xAA