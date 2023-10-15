; *******************************************************
; Pages.inc
; - Define variables
; 
%ifndef PAGING_INC_
%define PAGING_INC_
PMode_Setup_Paging:
    cld
    mov edi,0x70000
    xor eax,eax
    mov ecx,0x10000/4
    rep stosd
    
    mov dword[0x70000],0x71007
    mov dword[0x71000],10000111b
    ;-------------------------------;
	;   Disable Paging	            ;
	;-------------------------------;
    MOV     eax, cr0                                   ; Set the A-register to control register 0.
    
    AND     eax, 01111111111111111111111111111111b     ; Clear the PG-bit, which is bit 31.
    
    MOV     cr0, eax                                   ; Set control register 0 to the A-register.
    ;-------------------------------;
	;   Re-Enable 2MB Paging        ;
	;-------------------------------;
    MOV     edi, 0x1000    ; Set the destination index to 0x1000.
    
    MOV     cr3, edi       ; Set control register 3 to the destination index.
    
    XOR     eax, eax       ; Nullify the A-register.
    
    MOV     ecx, 4096      ; Set the C-register to 4096.
    
    REP     stosd          ; Clear the memory.
    
    MOV     edi, cr3       ; Set the destination index to control register 3.
    
    MOV     DWORD [edi], 0x2003      ; Set the uint32_t at the destination index to 0x2003.
    
    ADD     edi, 0x1000              ; Add 0x1000 to the destination index.
    
    MOV     DWORD [edi], 0x3003      ; Set the uint32_t at the destination index to 0x3003.
    
    ADD     edi, 0x1000              ; Add 0x1000 to the destination index.
    
    MOV     DWORD [edi], 0x4003      ; Set the uint32_t at the destination index to 0x4003.
    
    ADD     edi, 0x1000              ; Add 0x1000 to the destination index.
    
    MOV     ebx, 0x00000003          ; Set the B-register to 0x00000003.
    
    MOV     ecx, 512                 ; Set the C-register to 512.

.SetEntry:
    MOV     DWORD [edi], ebx         ; Set the uint32_t at the destination index to the B-register.
    
    ADD     ebx, 0x1000              ; Add 0x1000 to the B-register.
    
    ADD     edi, 8                   ; Add eight to the destination index.
    
    LOOP    .SetEntry                ; Set the next entry.

    MOV     eax,cr4                  ; Set the A-register to control register 4.
    
    OR  eax,(1 << 5)                 ; Set the PAE-bit, which is the 6th bit (bit 5).
    
    MOV     cr4,eax                  ; Set control register 4 to the A-register.

    MOV     ecx, 0xC0000080          ; Set the C-register to 0xC0000080, which is the EFER MSR.
    
    RDMSR                            ; Read from the model-specific register.
    
    OR  eax, (1 << 8)                ; Set the LM-bit which is the 9th bit (bit 8).
    
    WRMSR                            ; Write to the model-specific register.

    MOV     eax, cr0                 ; Set the A-register to control register 0.
    
    OR  eax, (1 << 31)               ; Set the PG-bit, which is the 32nd bit (bit 31).
    
    MOV     cr0, eax                 ; Set control register 0 to the A-register.
    ret
%endif