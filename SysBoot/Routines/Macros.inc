%ifndef __MACROS_INC
%define __MACROS_INC

%macro SAVE_STATE 0
    PUSH    r15
    
    PUSH    r14
    
    PUSH    r13
    
    PUSH    r12
    
    PUSH    r11
    
    PUSH    r10
    
    PUSH    r9
    
    PUSH    r8

    PUSH    rax
    
    PUSH    rcx
    
    PUSH    rdx
    
    PUSH    rbx
    
    PUSH    rsp
    
    PUSH    rbp
    
    PUSH    rsi
    
    PUSH    rdi
%endmacro

%macro RESTORE_STATE 0
    ; Restore registers
    POP     rdi
    
    POP     rsi
    
    POP     rbp
    
    ADD     rsp, 8 ; skip rsp
    
    POP     rbx
    
    POP     rdx
    
    POP     rcx
    
    POP     rax

    POP     r8
    
    POP     r9
    
    POP     r10
    
    POP     r11
    
    POP     r12
    
    POP     r13
    
    POP     r14
    
    POP     r15

	; Cleanup irq & error code from stack
    ADD     rsp, 0x10
%endmacro

%endif