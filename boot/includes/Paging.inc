; *******************************************************
; Pages.inc
; - Define variables
; 
%ifndef PAGING_INC_
%define PAGING_INC_
section .bss
align 4096
pageDir resd 1024
firstPageTable resd 1024

section .text
populateFirstTablePage:
    mov esi, first_page_table
    xor ecx, ecx
populateTables:
    mov eax, ecx
    shl eax, 12
    or eax, 3
    mov [esi + ecx * 4], eax
    inc ecx
    cmp ecx, 1024
    jl populateTables
    ret

addPageTableToDir:
    mov esi, pageDir
    mov eax, firstPageTable
    or eax, 3
    mov [esi], eax
    ret

; Load the page directory into CR3
loadPageDir:
    mov eax, pageDir
    mov cr3, eax
    ret

; Enable paging
enablePaging:
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret
%endif