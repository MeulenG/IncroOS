; Interrupt Service Routines
; This file contains the low-level ISR stubs for handling interrupts

BITS 64

section .text

; External C interrupt handler
extern interrupt_handler

; Common ISR stub that saves state and calls the C handler
; This is called by all ISR stubs after pushing interrupt number and error code
isr_common_stub:
    ; Save all general purpose registers
    ; Push in reverse order so they appear in the correct order in memory
    ; (stack grows downward, so last pushed = first in memory)
    push rdi
    push rsi
    push rbp
    push rsp
    push rbx
    push rdx
    push rcx
    push rax
    
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    ; Call C interrupt handler with pointer to interrupt frame
    ; The structure is now properly aligned with the stack layout
    mov rdi, rsp
    call interrupt_handler
    
    ; Restore all registers (in reverse order of push)
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    
    pop rax
    pop rcx
    pop rdx
    pop rbx
    add rsp, 8      ; Skip saved rsp
    pop rbp
    pop rsi
    pop rdi
    
    ; Clean up error code and interrupt number (2 qwords = 16 bytes)
    add rsp, 16
    
    ; Return from interrupt
    iretq

; Macro to create ISR stub without error code
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push qword 0            ; Push dummy error code
    push qword %1           ; Push interrupt number
    jmp isr_common_stub
%endmacro

; Macro to create ISR stub with error code (CPU pushes error code)
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    ; CPU already pushed error code
    push qword %1           ; Push interrupt number
    jmp isr_common_stub
%endmacro

; Macro to create IRQ stub
%macro IRQ 2
global irq%1
irq%1:
    push qword 0            ; Push dummy error code
    push qword %2           ; Push interrupt number (32 + IRQ number)
    jmp isr_common_stub
%endmacro

; CPU Exception ISRs (0-31)
ISR_NOERRCODE 0     ; Divide By Zero
ISR_NOERRCODE 1     ; Debug
ISR_NOERRCODE 2     ; Non Maskable Interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Overflow
ISR_NOERRCODE 5     ; Bound Range Exceeded
ISR_NOERRCODE 6     ; Invalid Opcode
ISR_NOERRCODE 7     ; Device Not Available
ISR_ERRCODE   8     ; Double Fault (has error code)
ISR_NOERRCODE 9     ; Coprocessor Segment Overrun
ISR_ERRCODE   10    ; Invalid TSS (has error code)
ISR_ERRCODE   11    ; Segment Not Present (has error code)
ISR_ERRCODE   12    ; Stack-Segment Fault (has error code)
ISR_ERRCODE   13    ; General Protection Fault (has error code)
ISR_ERRCODE   14    ; Page Fault (has error code)
ISR_NOERRCODE 15    ; Reserved
ISR_NOERRCODE 16    ; x87 Floating Point Exception
ISR_ERRCODE   17    ; Alignment Check (has error code)
ISR_NOERRCODE 18    ; Machine Check
ISR_NOERRCODE 19    ; SIMD Floating Point Exception
ISR_NOERRCODE 20    ; Virtualization Exception
ISR_ERRCODE   21    ; Control Protection Exception (has error code)
ISR_NOERRCODE 22    ; Reserved
ISR_NOERRCODE 23    ; Reserved
ISR_NOERRCODE 24    ; Reserved
ISR_NOERRCODE 25    ; Reserved
ISR_NOERRCODE 26    ; Reserved
ISR_NOERRCODE 27    ; Reserved
ISR_NOERRCODE 28    ; Hypervisor Injection Exception
ISR_ERRCODE   29    ; VMM Communication Exception (has error code)
ISR_ERRCODE   30    ; Security Exception (has error code)
ISR_NOERRCODE 31    ; Reserved

; Hardware IRQs (32-47) - mapped from IRQ 0-15
IRQ 0,  32          ; Timer (PIT)
IRQ 1,  33          ; Keyboard
IRQ 2,  34          ; Cascade (never raised)
IRQ 3,  35          ; COM2
IRQ 4,  36          ; COM1
IRQ 5,  37          ; LPT2
IRQ 6,  38          ; Floppy
IRQ 7,  39          ; LPT1
IRQ 8,  40          ; RTC
IRQ 9,  41          ; Free
IRQ 10, 42          ; Free
IRQ 11, 43          ; Free
IRQ 12, 44          ; PS/2 Mouse
IRQ 13, 45          ; FPU
IRQ 14, 46          ; Primary ATA
IRQ 15, 47          ; Secondary ATA

; Mark stack as non-executable (fix linker warning)
section .note.GNU-stack noalloc noexec nowrite progbits
