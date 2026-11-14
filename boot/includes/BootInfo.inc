; *******************************************************
; BootInfo.inc
; - The block stage2 fills in for the kernel, passed to kMain in RDI.
;   Keep the layout in sync with kernel/boot_info.h.
; *******************************************************
%ifndef BOOT_INFO_INC_
%define BOOT_INFO_INC_
bits 16

BOOT_INFO_MAGIC             equ 0x52434E49  ; "INCR"
BOOT_INFO_VERSION           equ 1
BOOT_INFO_MAX_MEMORY_ENTRIES equ 32
E820_ENTRY_SIZE             equ 24

struc BootInfo
    .magic:                 resd 1
    .version:               resd 1
    .size:                  resd 1
    .boot_drive:            resb 1
    .reserved:              resb 3
    .kernel_address:        resq 1
    .kernel_size:           resq 1
    .memory_map_count:      resd 1
    .memory_map_entry_size: resd 1
    .memory_map:            resb E820_ENTRY_SIZE * BOOT_INFO_MAX_MEMORY_ENTRIES
    .vbe_mode_info:         resb 256
endstruc

BootInfoBlock: times BootInfo_size db 0

;*******************************************
; BuildBootInfo()
;	- Fills BootInfoBlock. Call after the kernel
;	  is loaded and the VBE mode info is at 0x7000.
;*******************************************
BuildBootInfo:
    pushad
    push    es
    xor     ax, ax
    mov     es, ax

    mov     dword [BootInfoBlock + BootInfo.magic], BOOT_INFO_MAGIC
    mov     dword [BootInfoBlock + BootInfo.version], BOOT_INFO_VERSION
    mov     dword [BootInfoBlock + BootInfo.size], BootInfo_size
    mov     al, byte [bPhysicalDriveNum]
    mov     byte [BootInfoBlock + BootInfo.boot_drive], al
    mov     dword [BootInfoBlock + BootInfo.kernel_address], 0x100000
    mov     eax, dword [KernelSize]
    mov     dword [BootInfoBlock + BootInfo.kernel_size], eax
    mov     dword [BootInfoBlock + BootInfo.memory_map_entry_size], E820_ENTRY_SIZE

    ; Copy the VBE mode info, 0x7000 is only scratch space
    mov     si, 0x7000
    mov     di, BootInfoBlock + BootInfo.vbe_mode_info
    mov     cx, 256
    cld
    rep     movsb

    ; E820 memory map: each int 15h call returns one entry in ES:DI, EBX is 0 after the last one
    xor     ebx, ebx
    xor     bp, bp
    mov     di, BootInfoBlock + BootInfo.memory_map
.next_entry:
    mov     eax, 0xE820
    mov     edx, 0x534D4150             ; "SMAP"
    mov     ecx, E820_ENTRY_SIZE
    mov     dword [es:di + 20], 1       ; ACPI 3 attributes: valid, in case the BIOS fills only 20 bytes
    int     0x15
    jc      .done                       ; not supported, or past the last entry
    cmp     eax, 0x534D4150
    jne     .done
    ; skip entries with a zero length
    mov     eax, dword [es:di + 8]
    or      eax, dword [es:di + 12]
    jz      .skip
    inc     bp
    add     di, E820_ENTRY_SIZE
    cmp     bp, BOOT_INFO_MAX_MEMORY_ENTRIES
    jae     .done
.skip:
    test    ebx, ebx
    jnz     .next_entry
.done:
    mov     word [BootInfoBlock + BootInfo.memory_map_count], bp

    pop     es
    popad
    ret

%endif
