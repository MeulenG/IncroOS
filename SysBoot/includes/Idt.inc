%ifndef IDT_INC_
%define IDT_INC_

%ifdef REAL_MODE
BITS 16
;*******************************************
; InstallIDT()
;	- Install our IDT
;*******************************************
InstallIDT:
    lidt [idt_real]
    ret

;*******************************************
; Interrupt Descriptor Table (LDT)
;*******************************************

idt_real:
	dw 0x3ff		; 256 entries, 4b each = 1K
	dd 0			; Real Mode IVT @ 0x0000
%else
BITS 64

; Long mode IDT setup goes here

Idt:
    %rep 256
        ; Entry format for long mode IDT
        dw 0
        dw 0x8
        db 0
        db 0x8e
        dw 0
        dd 0
        dd 0
    %endrep

IdtLen: equ $-Idt

IdtPtr: dw IdtLen
        dq Idt

; Function to load the long mode IDT
InstallIDT64:
    lidt [IdtPtr]
    ret

%endif

%endif