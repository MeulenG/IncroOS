[BITS 16]
[ORG 0X600]
start:
    cli
    ; set to 0 before setting stack
    xor ax, ax
    mov ss, ax
	mov	ds, ax
	mov	es, ax
	mov	ax, 0x7C00
	mov	sp, ax
    cld

    // Copy 512 bytes from 0x7C00 to 0x0600
    mov si, 0x7C00
    mov di, 0x0600
    mov cx, 512
    cld

    rep movsb

    jmp 0x0000:relocated_address


relocated_address:
    ; 0x00	1 byte	Boot indicator bit flag: 0 = no, 0x80 = bootable (or "active")
    ; 0x01	1 byte	Starting head
    ; 0x02	6 bits	Starting sector (Bits 6-7 are the upper two bits for the Starting Cylinder field.)
    ; 0x03	10 bits	Starting Cylinder
    ; 0x04	1 byte	System ID
    ; 0x05	1 byte	Ending Head
    ; 0x06	6 bits	Ending Sector (Bits 6-7 are the upper two bits for the ending cylinder field)
    ; 0x07	10 bits	Ending Cylinder
    ; 0x08	4 bytes	Relative Sector (to start of partition -- also equals the partition's starting LBA value)
    ; 0x0C	4 bytes	Total Sectors in partition
    jmp $


times 510-($-$$) db 0
; Boot signature
dw 0xAA55