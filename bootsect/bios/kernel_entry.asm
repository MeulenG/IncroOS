[BITS   64]
[ORG    0x200000]

kernel_entry_start:
    mov byte[0xb8000], 'K'
    mov byte[0xb8001], 0xa

kernel_entry_end:
    hlt
    jmp kernel_entry_end