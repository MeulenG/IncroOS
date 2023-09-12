; *************************
; General x86 Real Mode Memory Map:
;   - 0x00000000 - 0x000003FF - Real Mode Interrupt Vector Table
;   - 0x00000400 - 0x000004FF - BIOS Data Area
;   - 0x00000500 - 0x00007BFF - Unused
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
BITS	16

ORG     0x7E00
main_Stage2: JMP Stage2_Main


;*******************************************************
;	Defines
;*******************************************************
%define DATA_SEGMENT    0x10

;*******************************************************
;	Preprocessor directives 16-BIT MODE
;*******************************************************
%include "../Routines/asmlib16.inc"

;*******************************************************
;	Preprocessor Descriptor Tables
;*******************************************************
%include "../Routines/Gdt.inc"
%include "../Routines/Idt.inc"

;*******************************************************
;	Preprocessor A20
;*******************************************************
%include "../Routines/A20.inc"

;******************************************************
;	ENTRY POINT For STAGE 2
;******************************************************
Stage2_Main:
	; Clear Interrupts
    CLI
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    XOR     ax, ax
    
    MOV	    ds, ax
    
    MOV	    es, ax
    
    MOV	    fs, ax
    
    MOV	    gs, ax

    MOV	    ss, ax
    
    MOV	    ax, 0x7C00
    
    MOV	    sp, ax
    ; Enable Interrupts
    STI
    ; Save Drive Number in DL
    MOV     [bPhysicalDriveNum],dl
    ; Time to check whether or not your processor supports CPUID
    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21) in
    ; the FLAGS register. If we can flip it, CPUID is available.
    ; Copy FLAGS in to EAX via stack
    PUSHFD
    
    POP     eax
 
    ; Copy to ECX as well for comparing later on
    MOV     ecx, eax
 
    ; Flip the ID bit
    XOR     eax, ( 1 << 21 )
 
    ; Copy EAX to FLAGS via the stack
    PUSH    eax
    
    POPFD
 
    ; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    PUSHFD
    
    POP     eax
 
    ; Restore FLAGS from the old version stored in ECX (i.e. flipping the ID bit
    ; back if it was ever flipped).
    PUSH    ecx
    
    POPFD
 
    ; Compare EAX and ECX. If they are equal then that means the bit wasn't
    ; flipped, and CPUID isn't supported.
    XOR     eax, ecx
    
    JZ  DEATHSCREEN.NoCPUID
    ; "cpuid" retrieve the information about your cpu
    ; eax=0x80000000: Get Highest Extended Function Implemented (only in long mode)
    ; 0x80000000 = 2^31
    MOV     eax,0x80000000
    
    CPUID
    ; Long mode can only be detected using the extended functions of CPUID (> 0x80000000)
    ; It is less, there is no long mode.
    CMP     eax,0x80000001
    ; if eax < 0x80000001 => CF=1 => jb
    ; jb, jump if below for unsigned number
    ; (jl, jump if less for signed number)
    JB      DEATHSCREEN.NoLongMode
    ; eax=0x80000001: Extended Processor Info and Feature Bits
    MOV     eax,0x80000001
    CPUID
    ; check if long mode is supported
    ; test => edx & (1<<29), if result=0, CF=1, then jump
    ; long mode is at bit-29
;    TEST    edx,( 1 << 29 ) ; We test to check whether it supports long mode
    ; jz, jump if zero => CF=1
;    JZ      DEATHSCREEN.NoLongMode
    ; check if 1g huge page support
    ; test => edx & (1<<26), if result=0, CF=1, then jump
    ; Gigabyte pages is at bit-26
;    TEST    edx,( 1 << 26 )
    ; jz, jump if zero => CF=1
;    JZ      DEATHSCREEN.NoLongMode

;    MOV     eax, 0x7
    
;    XOR     ecx, ecx
    
;    CPUID
    
;    TEST    ecx, ( 1 << 16 )
    
;    JNZ     DEATHSCREEN.5_level_paging

;******************************************************
;	Fat32 Routine
;******************************************************
FixCS:
	; Step 1. Calculate FAT32 Data Sector
	xchg bx, bx
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
			xchg bx, bx
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
        	jmp 	LoadFile

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
	jmp 	0x0:0xA000

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



;Most Modern Computers already have the A20 line set from the get-go, but if not then we enable it through BIOS
SetA20:
    ;-------------------------------;
	;   Enable A20 Line		        ;
	;-------------------------------;
    CALL EnableA20_Bios
    
;    MOV         si, msgA20
    
;    CALL        Puts16

SetVideoMode:
    ;-------------------------------;
	;   Set Video Mode  	        ;
	;-------------------------------;
    MOV     ax, 3

    INT     0x10

;    MOV         si, msgVideoMode
    
;    CALL        Puts16
    
    CLI
    ;-------------------------------;
	;   Install our GDT		        ;
	;-------------------------------;
    LGDT    [gdt_descriptor]
    
;    MOV         si, msggdt
    
;    CALL        Puts16
    ;-------------------------------;
	;   Install our IDT		        ;
	;-------------------------------;
    LIDT    [idt_real]
    
;    MOV         si, msgidt
    
;    CALL        Puts16
	;-------------------------------;
	;   Get Ready For PMode		    ;
	;-------------------------------;
    MOV     eax,cr0             ; Set the A-register to control register 0.
    
    OR  eax, (1 << 0)           ; Set The PM-bit, which is the 0th bit.
    
    MOV     cr0,eax             ; Set control register 0 to the A-register.

    JMP 8:ProtectedMode_Stage3  ; Get outta this cursed Real Mode

DEATHSCREEN:
    MOV         si, ErrorMsg
    
    CALL        Puts16                          ; Error message
    
    MOV         ah, 0x00
    
    INT         0x16                            ; SMACK YOUR ASS ON THAT KEYBOARD AGAIN
    
    INT         0x19                            ; Reboot and try again, bios uses int 0x19 to find a bootable device

    .NoCPUID:
        MOV     si, ErrorMsgCPUID

        CALL    Puts16

        MOV         ah, 0x00
    
        INT         0x16                            ; SMACK YOUR ASS ON THAT KEYBOARD AGAIN
    
        INT         0x19                            ; Reboot and try again, bios uses int 0x19 to find a bootable device

    .NoLongMode:
        MOV     si, ErrorMsgLongMode

        CALL    Puts16
        
        MOV         ah, 0x00
    
        INT         0x16                            ; SMACK YOUR ASS ON THAT KEYBOARD AGAIN
    
        INT         0x19                            ; Reboot and try again, bios uses int 0x19 to find a bootable device

    .NoKernel:
        MOV     si, ErrorMsgKernel

        CALL    Puts16

        MOV         ah, 0x00
    
        INT         0x16                            ; SMACK YOUR ASS ON THAT KEYBOARD AGAIN
    
        INT         0x19                            ; Reboot and try again, bios uses int 0x19 to find a bootable device
    
    .5_level_paging:
        MOV     si, ErrorMsgLevel5Paging

        CALL    Puts16
        
        MOV         ah, 0x00
    
        INT         0x16                            ; SMACK YOUR ASS ON THAT KEYBOARD AGAIN
    
        INT         0x19                            ; Reboot and try again, bios uses int 0x19 to find a bootable device

ALIGN   32
BITS    32
;*******************************************************
;	Preprocessor directives 32-BIT MODE
;*******************************************************
%include "../Routines/asmlib32.inc"
;******************************************************
;	ENTRY POINT For STAGE 3
;******************************************************
ProtectedMode_Stage3:
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    MOV     ax, DATA_SEGMENT
    
    MOV     ds, ax
    
    MOV     es, ax
    
    MOV     ss, ax
    
    MOV     esp, 0x7c00

    CLD
    
    MOV     edi, 0x80000
    
    XOR     eax, eax
    
    MOV     ecx, 0x10000/4
    
    REP     stosd
    
    MOV     dword[0x80000], 0x81007
    
    MOV     dword[0x81000], 10000111b

	;---------------------------------------;
	;   Protected Mode Reached	            ;
	;---------------------------------------;
	CALL	ClrScr32
	
    MOV		ebx, PrepareMsg64
	
    CALL	Puts32
	
	MOV		ebx, msgStart
	
    CALL	Puts32

	MOV		ebx, msgStageTwo
	
    CALL	Puts32

	MOV		ebx, LoadingMsg
	
    CALL	Puts32

    MOV		ebx, msgpmode
	
    CALL	Puts32

    ;-------------------------------;
	;   Install our new GDT		    ;
	;-------------------------------;
    LGDT    [gdt_descriptor_64]
    
    MOV     ebx, msggdt
    
    CALL    Puts32
	;-------------------------------;
	;   Messages From Real Mode	    ;
	;-------------------------------;
	MOV 	ebx, msgA20

	CALL 	Puts32

	MOV 	ebx, msgidt

	CALL 	Puts32

	MOV 	ebx, msgKernelLoaded

	CALL 	Puts32

	MOV 	ebx, msgVideoMode

	CALL 	Puts32
    ;-------------------------------;
	;   Get Ready For Long Mode		;
	;-------------------------------;
	MOV		ebx, msglongmode
	
    CALL	Puts32

    MOV     ebx, PrepareMsgKernel64

    CALL    Puts32
    ;-------------------------------;
	;   Disable Paging	            ;
	;-------------------------------;
    MOV     eax, cr0                                   ; Set the A-register to control register 0.
    
    AND     eax, 01111111111111111111111111111111b     ; Clear the PG-bit, which is bit 31.
    
    MOV     cr0, eax                                   ; Set control register 0 to the A-register.
    ;-------------------------------;
	;   Re-Enable 2MB Paging        ;
	;-------------------------------;
    MOV     edi, 0x1000    ; Set the destination index to 0x1000.
    
    MOV     cr3, edi       ; Set control register 3 to the destination index.
    
    XOR     eax, eax       ; Nullify the A-register.
    
    MOV     ecx, 4096      ; Set the C-register to 4096.
    
    REP     stosd          ; Clear the memory.
    
    MOV     edi, cr3       ; Set the destination index to control register 3.
    
    MOV     DWORD [edi], 0x2003      ; Set the uint32_t at the destination index to 0x2003.
    
    ADD     edi, 0x1000              ; Add 0x1000 to the destination index.
    
    MOV     DWORD [edi], 0x3003      ; Set the uint32_t at the destination index to 0x3003.
    
    ADD     edi, 0x1000              ; Add 0x1000 to the destination index.
    
    MOV     DWORD [edi], 0x4003      ; Set the uint32_t at the destination index to 0x4003.
    
    ADD     edi, 0x1000              ; Add 0x1000 to the destination index.
    
    MOV     ebx, 0x00000003          ; Set the B-register to 0x00000003.
    
    MOV     ecx, 512                 ; Set the C-register to 512.

.SetEntry:
    MOV     DWORD [edi], ebx         ; Set the uint32_t at the destination index to the B-register.
    
    ADD     ebx, 0x1000              ; Add 0x1000 to the B-register.
    
    ADD     edi, 8                   ; Add eight to the destination index.
    
    LOOP    .SetEntry                ; Set the next entry.

    MOV     eax,cr4                  ; Set the A-register to control register 4.
    
    OR  eax,(1 << 5)                 ; Set the PAE-bit, which is the 6th bit (bit 5).
    
    MOV     cr4,eax                  ; Set control register 4 to the A-register.

    MOV     ecx, 0xC0000080          ; Set the C-register to 0xC0000080, which is the EFER MSR.
    
    RDMSR                            ; Read from the model-specific register.
    
    OR  eax, (1 << 8)                ; Set the LM-bit which is the 9th bit (bit 8).
    
    WRMSR                            ; Write to the model-specific register.

    MOV     eax, cr0                 ; Set the A-register to control register 0.
    
    OR  eax, (1 << 31)               ; Set the PG-bit, which is the 32nd bit (bit 31).
    
    MOV     cr0, eax                 ; Set control register 0 to the A-register.

    JMP     8:Stage4_Long_Mode

PEnd:
    HLT
    
    JMP     PEnd
ALIGN   64
BITS    64
;*******************************************************
;	Preprocessor directives 64-BIT MODE
;*******************************************************
%include "../Routines/asmlib64.inc"
;******************************************************
;	ENTRY POINT For STAGE 5
;******************************************************
Stage4_Long_Mode:
    ;-------------------------------;
	;   Disable Interrupts			;
	;-------------------------------;
    CLI                             ; Clear The Interrupt Flag
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;    
    MOV     ax, DATA_SEGMENT
    
    MOV     ds, ax
    
    MOV     es, ax
    
    MOV     fs, ax
    
    MOV     gs, ax
    
    MOV     ss, ax
 
    JMP 	0xA000
    
LEnd:
    HLT
    
    JMP     LEnd

MAIN_LONG:


;*******************************************************
;	Data Section
;*******************************************************
; Reserved
dReserved0					dd		0 	;FirstDataSector

dReserved1					dd		0 	;ReadCluster

dReserved2					dd 		0 	;ReadCluster

syskrnldr					db 		"KRNLDR  SYS"

bPhysicalDriveNum			DB		0

msgA20                      DB  0x0A, 0x0D, "Enabling A20 Gate", 0x00

msggdt                      DB  0x0A, 0x0D, "Installing GDT", 0x00

msgidt                      DB  0x0A, 0x0D, "Installing IDT", 0x00

msgKernelLoaded             DB  0x0A, 0x0D, "Loading Kernel", 0x00

msgVideoMode                DB  0x0A, 0x0D, "Video Mode Set", 0x00

PrepareMsg64:				DB  0x0A, 0x0D,"Preparing To Load 64-Bit Operating System...",  0x00

PrepareMsgKernel64:			DB  0x0A, 0x0D,"Handing Control To The Kernel, Stand By...",  0x00

msgStart                    DB  0x0D,0x0A, "Boot Loader starting (0x7C00)....", 0x00

msgStageTwo                 DB  0x0D, 0x0A, "Jumping to stage2 (0x7E00)", 0x00


ReadPacket:                 times 16 db 0

LoadingMsg                  DB 0x0D, 0x0A, "Stage 2 Sucessfully Loaded", 0x00

msgpmode:                   DB  0x0A, 0x0A, 0x0A, "               <[OMOS Protected Mode]>         "
                            DB  0x0A, 0x0A   		"            Welcome To Protected Mode", 0

msglongmode:                DB  0x0A, 0x0A, 0x0A, "               <[OMOS Long Mode]>"
                            DB  0x0A, 0x0A   		"            Welcome To 64-Bit Mode Long Mode", 0
;*******************************************************
;	Data Error Section
;*******************************************************
ErrorMsg                    DB  "Hahahaha, Fuck you, but at Stage2"

ErrorMsgLongMode            DB  "Long Mode Is Not Supported On This Machine"

ErrorMsgKernel              DB  "Unable To Load Kernel"

ErrorMsgA20                 DB  "Unable To Set The A20 Line"

ErrorMsgCPUID               DB  "Processor does not support CPUID"

ErrorMsgLevel5Paging        DB  "Level 5 Paging Not Avilable"

ErrorFixCS:  DB "FixCS error", 0x00

ErrorReadSector: DB "ReadSector error", 0x00

