; Serial Port Driver - Assembly Implementation
; COM1 base port: 0x3F8

section .text

global serial_init
global serial_putchar

; COM1 port addresses
%define COM1_PORT       0x3F8
%define COM1_DATA       COM1_PORT       ; Data register
%define COM1_IER        COM1_PORT + 1   ; Interrupt Enable Register
%define COM1_FIFO       COM1_PORT + 2   ; FIFO Control Register
%define COM1_LCR        COM1_PORT + 3   ; Line Control Register
%define COM1_MCR        COM1_PORT + 4   ; Modem Control Register
%define COM1_LSR        COM1_PORT + 5   ; Line Status Register

; serial_init - Initialize the serial port (COM1)
; No parameters
; No return value
serial_init:
    push rax
    push rdx
    
    ; Disable all interrupts
    mov dx, COM1_IER
    xor al, al
    out dx, al
    
    ; Enable DLAB (set bit 7 of LCR) to set baud rate divisor
    mov dx, COM1_LCR
    mov al, 0x80
    out dx, al
    
    ; Set divisor to 3 (lo byte) 38400 baud
    mov dx, COM1_DATA
    mov al, 0x03
    out dx, al
    
    ; Set divisor to 0 (hi byte)
    mov dx, COM1_IER
    xor al, al
    out dx, al
    
    ; 8 bits, no parity, one stop bit (0x03)
    mov dx, COM1_LCR
    mov al, 0x03
    out dx, al
    
    ; Enable FIFO, clear them, with 14-byte threshold
    mov dx, COM1_FIFO
    mov al, 0xC7
    out dx, al
    
    ; IRQs enabled, RTS/DSR set
    mov dx, COM1_MCR
    mov al, 0x0B
    out dx, al
    
    pop rdx
    pop rax
    ret

; serial_putchar - Write a character to the serial port
; Parameters:
;   rdi - character to write (first argument in x86-64 calling convention)
; No return value
serial_putchar:
    push rax
    push rdx
    
    mov al, dil             ; Get the character from first argument
    
    ; Wait for transmit buffer to be empty
.wait:
    mov dx, COM1_LSR
    in al, dx
    test al, 0x20           ; Check if transmit buffer is empty (bit 5)
    jz .wait
    
    ; Send the character
    mov al, dil
    mov dx, COM1_DATA
    out dx, al
    
    pop rdx
    pop rax
    ret
