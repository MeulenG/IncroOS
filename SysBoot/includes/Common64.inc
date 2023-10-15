%ifndef __COMMON64_INC
%define __COMMON64_INC

BITS 64

SetStack64:
    ;--------------------------------;
    ;   Setup segments and stack	   ;
    ;--------------------------------;
    ; Setting up the Stack Segment selector and stack pointer
	mov	ax, DATA_SEGMENT	; set data segments to data selector (0x10)
	mov	ds, ax
	mov	ss, ax
	mov	es, ax
	mov	esp, 90000h		; stack begins from 90000h
    ; Jump to the next part of the code
    jmp Continue_Part3

%endif