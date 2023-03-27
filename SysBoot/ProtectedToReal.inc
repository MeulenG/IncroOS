%ifndef __PROTECTEDTOREAL_INC
%define __PROTECTEDTOREAL_INC

[BITS 32]

idt_real:
	dw 0x3ff		; 256 entries, 4b each = 1K
	dd 0			; Real Mode IVT @ 0x0000
 
savcr0:
	dd 0			; Storage location for pmode CR0.

ProtectedToRealMode:
;-------------------------------------------------------
; Disable Interrupts
;-------------------------------------------------------
    cli

;-------------------------------------------------------
; Need 16-bit Protected Mode GDT entries!
;-------------------------------------------------------
    mov     eax, DATASEL16

    mov     ds, eax

    mov     es, eax

    mov     fs, eax

    mov     gs, eax

    mov     ss, eax

;-------------------------------------------------------
; Disable Paging
;-------------------------------------------------------
    mov     eax, cr0

    mov     [savcr0], eax

    and     eax, 0x7FFFFFFe

    mov     cr0, eax

    jmp 0:SwitchToRealMode

SwitchToRealMode:
;-------------------------------------------------------
; Pick a stack pointer and set the segment registers
;-------------------------------------------------------
	mov     sp, 0x7E00
	
    mov     ax, 0
	
    mov     ds, ax
	
    mov     es, ax
	
    mov     fs, ax
	
    mov     gs, ax
	
    mov     ss, ax
	
    lidt    [idt_real]

;-------------------------------------------------------
; Restore interrupts -- be careful, unhandled int's will kill it.
;-------------------------------------------------------
    sti

%endif