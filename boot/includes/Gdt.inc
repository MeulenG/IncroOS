; *******************************************************
; Gdt.inc
; - Contains routines for setting up GDT
; 
%ifndef GDT_INC_
%define GDT_INC_
; ********************************
; GdtInstall
; ********************************
GdtInstall:
    lgdt [TemporaryGdt]
    ret

;*******************************************
; Global Descriptor Table
;*******************************************
StartOfGdt:
    dd 0                ; null descriptor
    dd 0 

; gdt code:	            ; code descriptor
    dw 0FFFFh           ; limit low
    dw 0                ; base low
    db 0                ; base middle
    db 10011010b        ; access
    db 11001111b        ; granularity
    db 0                ; base high

; gdt data:	            ; data descriptor
    dw 0FFFFh           ; limit low (Same as code)
    dw 0                ; base low
    db 0                ; base middle
    db 10010010b        ; access
    db 11001111b        ; granularity
    db 0                ; base high

; gdt code 16bit:	    ; code descriptor
    dw 0FFFFh           ; limit low
    dw 0                ; base low
    db 0                ; base middle
    db 10011010b        ; access
    db 00001111b        ; granularity
    db 0                ; base high

; gdt data 16bit:       ; data descriptor
    dw 0FFFFh           ; limit low (Same as code)
    dw 0                ; base low
    db 0                ; base middle
    db 10010010b        ; access
    db 00001111b        ; granularity
    db 0                ; base high

; gdt code 64bit:       ; code descriptor
    dw 0                ; limit low
    dw 0                ; base low
    db 0                ; base middle
    db 10011010b        ; access
    db 00100000b        ; granularity
    db 0                ; base high

; gdt data 64bit:       ; data descriptor
    dw 0                ; limit low (Same as code)
    dw 0                ; base low
    db 0                ; base middle
    db 10010010b        ; access
    db 00000000b        ; granularity
    db 0                ; base high
EndOfGdt:

; The actual Gdt Header
TemporaryGdt: 
    dw EndOfGdt - StartOfGdt - 1
    dd StartOfGdt

; The actual Gdt Header
TemporaryGdt64bit: 
    dw EndOfGdt - StartOfGdt - 1
    dq StartOfGdt
%endif