; *******************************************************
; Fat32Driver.inc
; - Contains routines for loading files on Fat32
; 
%ifndef FAT32DRIVER_INC_
%define FAT32DRIVER_INC_


;******************************************************
;	Fat32 Driver
;******************************************************

; Calculate FAT32 Data Sector for a given cluster
; Input:
;   - esi contains the starting cluster
; Output: 
;   - dReserved0 will contain the calculated data sector
CalculateDataSector:
    ; Clear eax for calculations
    xor     eax, eax
    
    ; Load necessary constants
    mov     al, [0x7C00 + 0x10]  ; bNumFats
    mov     ebx, dword [0x7C00 + 0x24]  ; SectorsPerFat32
    
    ; Perform calculations
    mul     ebx  ; al * ebx -> eax
    xor     ebx, ebx
    mov     bx, word [0x7C00 + 0x0E]  ; Reserved sectors
    add     eax, ebx
    
    ; Factor in the starting cluster
    add     eax, esi  ; eax += starting cluster
    
    ; Store the result
    mov     dword [dReserved0], eax
    ret


; Read FAT Table for a given cluster
; Input:
;   - esi contains the starting cluster
; Output:
;   - esi will contain the next cluster in the chain
ReadFATTable:
    ; Load FAT table location from BPB (BIOS Parameter Block)
    mov     esi, dword [0x7C00 + 0x2C]
    
    ; Calculate the FAT offset for the cluster
    lea     edi, [esi + 4 * esi]
    
    ; Read the FAT entry for the cluster into esi
    ; Assuming DS:SI points to FAT table in memory
    mov     esi, [ds:edi]
    
    ; Handle special values for end of cluster chain
    cmp     esi, 0x0FFFFFF8
    jae     .EndOfChain
    
    ret
    
    ; Label for special values indicating end of cluster chain
    .EndOfChain:
        ; Handle end of chain (you might want to set esi to a special value or raise an error)
        ret


; Read a cluster into memory
; Input:
;   - esi contains the cluster to read
;   - es:di points to the memory buffer
; Output:
;   - Memory buffer will contain the read data
ReadClusterIntoMemory:
    ; Calculate the LBA (Logical Block Address) for the cluster
    ; This usually involves some arithmetic based on the BPB (BIOS Parameter Block)
    ; For example, LBA = Cluster * SectorsPerCluster + FirstDataSector
    ; Store the result in a register or memory location
    
    ; Read the cluster into the buffer using BIOS interrupts or other methods
    
    ret


; Parse directory entries in a cluster
; Input:
;   - es:di points to the memory buffer containing the cluster
;   - ds:si points to the null-terminated string of the file name to search for
; Output:
;   - If found, esi will contain the starting cluster of the file
;   - If not found, set a flag or error code
ParseDirectoryEntries:
    ; Loop through each directory entry in the cluster
    .EntryLoop:
        ; Read the file name from the current directory entry
        ; Compare it with the target file name
        
        ; If the names match, read the starting cluster for the file into esi
        ; Set a flag or error code to indicate that the file was found
        
        ; Move to the next directory entry in the buffer
        
    ret

FixCS:
	; Step 1. Calculate FAT32 Data Sector
	xor		eax, eax
	mov 	al, [0x7C00 + 0x10]
	mov 	ebx, dword [0x7C00 + 0x24]
	mul 	ebx
	xor 	ebx, ebx
	mov 	bx, word [0x7C00 + 0x0E]
	add 	eax, ebx
	mov 	dword [dReserved0], eax

	; Step 2. Read FAT Table
	mov 	esi, dword [0x7C00 + 0x2C]

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
		mov 	si, syskrnldr
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
	mov 	bx, 0xA000

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
	jmp 	SetA20

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
	mov 	bl, byte [0x7C00 + 0x0D]
	mul 	bx
	add 	eax, dword [dReserved0]

	; Eax is now the sector of data
	pop 	bx
	mov 	cl, byte [0x7C00 + 0x0D]

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
        div     WORD [0x7C18]
        inc     dl ; adjust for sector 0
        mov     cl, dl ;Absolute Sector
        xor     dx, dx
        div     WORD [0x7C1A]
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

		add 	bx, word [0x7C00 + 0xB]
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
	div 	word [0x7C00 + 0x0B]
	add 	ax, word [0x7C00 + 0x0E]
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


%endif