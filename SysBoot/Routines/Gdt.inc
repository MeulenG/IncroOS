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
    lgdt [gdt_descriptor]
    ret


gdt_start:

gdt_null:           ; The mandatory null descriptor
    dd 0x0          ; dd = define double word (4 bytes)
    dd 0x0

gdt_code:           ; Code segment descriptor
    dw 0xffff       ; Limit (bites 0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10011010b    ; 1st flags, type flags
    db 11001111b    ; 2nd flags, limit (bits 16-19)
    db 0x0          ; Base (bits 24-31)

gdt_data:
    dw 0xffff       ; Limit (bites 0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10010010b    ; 1st flags, type flags
    db 11001111b    ; 2nd flags, limit (bits 16-19)
    db 0x0

gdt_end:            ; necessary so assembler can calculate gdt size below

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size

    dd gdt_start                ; Start adress of GDT

gdt_descriptor_64:
    dw gdt_end - gdt_start - 1  ; GDT size

    dq gdt_start                ; Start adress of GDT

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

%endif