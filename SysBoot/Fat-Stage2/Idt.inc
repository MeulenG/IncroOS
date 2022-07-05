%ifndef IDT_INC_
%define IDT_INC_
BITS    16

idt_real:
	dw 0x3ff		; 256 entries, 4b each = 1K
	dd 0			; Real Mode IVT @ 0x0000

%endif