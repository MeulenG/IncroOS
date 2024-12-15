; *******************************************************
; Pages.inc
; - Define variables
; 
%ifndef PAGING_INC_
%define PAGING_INC_
AllocatePageTable:
    push edi
    push VBOOT_MEMORY_TYPE_RECLAIM
    push 0x1000
    call MemoryAllocate
    add esp, 8
    cmp eax, 0
    jz .exit

    cld
    mov edx, eax  ; store for return value
    mov edi, eax
    xor eax, eax
    mov ecx, 0x400
    rep stosd
    mov eax, edx

    .exit:
        pop edi
        ret
    
%endif