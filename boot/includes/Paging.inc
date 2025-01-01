; *******************************************************
; Pages.inc
; - Define variables
; 
%ifndef PAGING_INC_
%define PAGING_INC_
section .bss
align 4096
page_directory resd 1024
first_page_table resd 1024

section .text
populate_first_page_table:
    mov esi, first_page_table
    xor ecx, ecx
populate_loop:
    mov eax, ecx
    shl eax, 12
    or eax, 3
    mov [esi + ecx * 4], eax
    inc ecx
    cmp ecx, 1024
    jl populate_loop
    ret

add_page_table_to_directory:
    mov esi, page_directory
    mov eax, first_page_table
    or eax, 3
    mov [esi], eax
    ret

; Load the page directory into CR3
loadPageDir:
    mov eax, page_directory
    mov cr3, eax
    ret

; Enable paging
enablePaging:
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret
%endif