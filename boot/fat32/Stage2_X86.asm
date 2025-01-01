; *************************
; General x86 Real Mode Memory Map:
;   - 0x00000000 - 0x000003FF - Real Mode Interrupt Vector Table
;   - 0x00000400 - 0x000004FF - BIOS Data Area
;   - 0x00000500 - 0x00007BFF - Kernel
;   - 0x00007C00 - 0x00007DFF - Stage 1 (512 Bytes)
;   - 0x00007E00 - 0x0009FFFF - Stage 2
;   - 0x000A0000 - 0x000BFFFF - Video RAM (VRAM) Memory
;   - 0x000B0000 - 0x000B7777 - Monochrome Video Memory
;   - 0x000B8000 - 0x000BFFFF - Color Video Memory
;   - 0x000C0000 - 0x000C7FFF - Video ROM BIOS
;   - 0x000C8000 - 0x000EFFFF - BIOS Shadow Area
;   - 0x000F0000 - 0x000FFFFF - System BIOS
; *************************
; *************************
;    Real Mode 16-Bit
; - Uses the native Segment:offset memory model
; - Limited to 1MB of memory
; - No memory protection or virtual memory
; *************************
bits	16

org     0x7E00
jmp Stage2_Entry

;*******************************************************
;	Preprocessor directives 16-BIT MODE
;*******************************************************
%include "../includes/stdio16.inc"
%include "../includes/Macros.inc"
%include "../includes/GlobalDefines.inc"
%include "../includes/cpu.inc"
%include "../includes/Gdt.inc"
%include "../includes/Idt.inc"
%include "../includes/A20.inc"

;*******************************************************
;	Data Section
;*******************************************************
%include "../includes/DataSection.inc"

;*******************************************************
;	STAGE 2 ENTRY POINT
;
;		-Store BIOS information
;		-Load Kernel
;		-Install GDT; go into protected mode (pmode)
;		-Jump to Stage 3
;*******************************************************
FixCS:
    ; Fix segment registers to 0
	xor 	ax, ax
	mov		ds, ax
	mov		es, ax

	; Set stack
	mov		ss, ax
	mov		ax, 0x7E00
	mov		sp, ax

	; Done, now we need interrupts again
	sti

	; Step 0. Save DL
	mov 	byte [bPhysicalDriveNum], dl

	; Step 1. Calculate FAT32 Data Sector
	xor		eax, eax
	mov 	al, [0x7C00 + 0x10]
	mov 	ebx, dword [0x7C00 + 0x24]
	mul 	ebx
	xor 	ebx, ebx
	mov 	bx, word [0x7C00 + 0x0E]
	add 	eax, ebx
	mov 	dword [0x7C00 + 0x34], eax

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

		; Step 3. Parse entries and look for kernel
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
	mov 	bx, 0x500

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
	jmp     CheckCPU

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
	add 	eax, dword [0x7C00 + 0x34]

	; Eax is now the sector of data
	pop 	bx
	mov 	cl, byte [0x7C00 + 0x0D]

	; Read
	call 	ReadSector

	; Save position
	mov 	word [0x7C00 + 0x3C], bx
	push 	es

	; Si still has cluster num, call next
	call 	GetNextCluster
	mov 	dword [0x7C00 + 0x38], esi

	; Restore
	pop 	es

	; Done
	popa
	mov 	bx, word [0x7C00 + 0x3C]
	mov 	esi, dword [0x7C00 + 0x38]
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
        div     WORD [0x7C00 + 0x18]
        inc     dl ; adjust for sector 0
        mov     cl, dl ;Absolute Sector
        xor     dx, dx
        div     WORD [0x7C00 + 0x1A]
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

		add 	bx, word [0x7C00 + 0x0B]
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

Stage2_Entry:
    ; Load File
    call    FixCS


CheckCPU:
    ; Is this CPU eligible?
    call    DetectCPU

;Most Modern Computers already have the A20 line set from the get-go, but if not then we enable it
SetA20:
    ;-------------------------------;
	;   Enable A20 Line		        ;
	;-------------------------------;
    call    EnableA20_KKbrd_Out

SetVideoMode:
    ;-------------------------------;
	;   Set Video Mode  	        ;
	;-------------------------------;
    mov     ax, 3
    int     0x10
    cli
    ;-------------------------------;
	;   Install our GDT		        ;
	;-------------------------------;
    call    GdtInstall
    
    ;-------------------------------;
	;   Install our IDT		        ;
	;-------------------------------;
    call    InstallIDT
    
	;-------------------------------;
	;   Get Ready For PMode		    ;
	;-------------------------------;
    mov     eax,cr0             ; Set the A-register to control register 0.
    
    OR      eax, (1 << 0)       ; Set The PM-bit, which is the 0th bit.
        
    mov     cr0,eax             ; Set control register 0 to the A-register.

    jmp     CODE_DESC:LoaderEntry32  ; Get outta this cursed Real Mode

ReadError:
End:
    hlt
    jmp End
ALIGN   32
BITS    32
;*******************************************************
;	Preprocessor directives 32-BIT MODE
;*******************************************************
%include "../includes/stdio32.inc"
%include "../includes/Paging.inc"
;%include "../includes/Gdt64.inc"
;******************************************************
;	ENTRY POINT For STAGE 3
;******************************************************
LoaderEntry32:
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
	xchg bx, bx

	mov ax, DATA_SEGMENT
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
    
	mov esi, 0x00000500
	mov edi, 0x00100000
	mov ecx, 4496
	rep movsb

	call populate_first_page_table
	call add_page_table_to_directory
	call loadPageDir
	call enablePaging

	


Continue_Part2:
    jmp 0x8:LoaderEntry64

ALIGN   64
BITS    64
;*******************************************************
;	Preprocessor directives 64-BIT MODE
;*******************************************************
%include "../includes/stdio64.inc"
;******************************************************
;	ENTRY POINT For STAGE 4
;******************************************************
LoaderEntry64:
    ;--------------------------------;
    ;   Setup segments and stack	 ;
    ;--------------------------------;
    ; Setting up the Stack Segment selector and stack pointer
	cli
	mov	ax, DATA_SEGMENT64
	mov	ds, ax
	mov	es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov	es, ax
	mov	rsp, 90000h		; stack begins from 90000h
	xchg bx, bx
    jmp Continue_Part3

Continue_Part3:
	jmp     KERNEL_BASE_ADDRESS