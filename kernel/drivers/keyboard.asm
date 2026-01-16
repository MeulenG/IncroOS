; Keyboard Driver - Assembly Port I/O Functions
; Provides low-level port I/O for keyboard controller

section .text

global inb
global outb

; inb - Read a byte from an I/O port
; Parameters:
;   rdi - port number (16-bit)
; Returns:
;   al - byte read from port
inb:
    push rdx
    mov dx, di          ; Move port number to dx
    in al, dx           ; Read byte from port into al
    pop rdx
    ret

; outb - Write a byte to an I/O port
; Parameters:
;   rdi - port number (16-bit)
;   rsi - byte value to write
; No return value
outb:
    push rax
    push rdx
    mov dx, di          ; Move port number to dx
    mov al, sil         ; Move byte value to al
    out dx, al          ; Write byte to port
    pop rdx
    pop rax
    ret

; Indicate that this code does not require an executable stack
section .note.GNU-stack noalloc noexec nowrite progbits
