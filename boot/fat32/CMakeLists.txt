project (boot-master)

enable_language (ASM_NASM)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/BOOT.SYS
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f bin -o ${CMAKE_BINARY_DIR}/BOOT.SYS Boot1.asm
    COMMENT "Compiling bootloader (Boot1)"
    DEPENDS Boot1.asm
)
add_custom_target(Boot ALL DEPENDS ${CMAKE_BINARY_DIR}/BOOT.SYS)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/STAGE2.SYS
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f bin -o ${CMAKE_BINARY_DIR}/STAGE2.SYS Stage2_X86.asm
    COMMENT "Compiling bootloader (stage2)"
    DEPENDS Stage2_X86.asm
)
add_custom_target(stage2 ALL DEPENDS ${CMAKE_BINARY_DIR}/STAGE2.SYS)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/Kernel_Entry.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f elf64 -o ${CMAKE_BINARY_DIR}/Kernel_Entry.o Kernel_Entry_X86.asm
    COMMENT "Compiling Kernel Entry"
    DEPENDS Kernel_Entry_X86.asm
)

add_custom_target(KernelEntry ALL DEPENDS ${CMAKE_BINARY_DIR}/Kernel_Entry.o)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/KRNLDR.ELF
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ld -o ${CMAKE_BINARY_DIR}/KRNLDR.ELF ${CMAKE_BINARY_DIR}/Kernel_Entry.o ${CMAKE_BINARY_DIR}/kernel.o ${CMAKE_BINARY_DIR}/terminal.o ${CMAKE_BINARY_DIR}/boot_service.o ${CMAKE_BINARY_DIR}/vesa_terminal.o ${CMAKE_BINARY_DIR}/serial.o ${CMAKE_BINARY_DIR}/keyboard.o ${CMAKE_BINARY_DIR}/vesa.o ${CMAKE_BINARY_DIR}/ata.o ${CMAKE_BINARY_DIR}/fat32.o ${CMAKE_BINARY_DIR}/pmm.o ${CMAKE_BINARY_DIR}/vmm.o ${CMAKE_BINARY_DIR}/kmalloc.o ${CMAKE_BINARY_DIR}/print.o ${CMAKE_BINARY_DIR}/string.o ${CMAKE_BINARY_DIR}/gdt.o ${CMAKE_BINARY_DIR}/idt.o ${CMAKE_BINARY_DIR}/interrupt_handler.o ${CMAKE_BINARY_DIR}/pic.o ${CMAKE_BINARY_DIR}/zstd_kernel.o -Ttext=0x100000 --entry=_start
    # Stage2 jumps to 0x100000, not to the ELF entry. The default linker script puts .text.unlikely
    # and friends before .text, so an object with those sections would silently move _start.
    COMMAND sh -c "nm ${CMAKE_BINARY_DIR}/KRNLDR.ELF | grep -qx '0000000000100000 T _start' || { echo 'KRNLDR.ELF: _start is not at 0x100000, where stage2 jumps' >&2; rm -f ${CMAKE_BINARY_DIR}/KRNLDR.ELF; exit 1; }"
    COMMENT "Linking Kernel to ELF"
    DEPENDS ${CMAKE_BINARY_DIR}/Kernel_Entry.o ${CMAKE_BINARY_DIR}/kernel.o ${CMAKE_BINARY_DIR}/terminal.o ${CMAKE_BINARY_DIR}/boot_service.o ${CMAKE_BINARY_DIR}/vesa_terminal.o ${CMAKE_BINARY_DIR}/serial.o ${CMAKE_BINARY_DIR}/keyboard.o ${CMAKE_BINARY_DIR}/vesa.o ${CMAKE_BINARY_DIR}/ata.o ${CMAKE_BINARY_DIR}/fat32.o ${CMAKE_BINARY_DIR}/pmm.o ${CMAKE_BINARY_DIR}/vmm.o ${CMAKE_BINARY_DIR}/kmalloc.o ${CMAKE_BINARY_DIR}/print.o ${CMAKE_BINARY_DIR}/string.o ${CMAKE_BINARY_DIR}/gdt.o ${CMAKE_BINARY_DIR}/idt.o ${CMAKE_BINARY_DIR}/interrupt_handler.o ${CMAKE_BINARY_DIR}/pic.o ${CMAKE_BINARY_DIR}/zstd_kernel.o
    VERBATIM
)

add_custom_target(KernelELF ALL DEPENDS ${CMAKE_BINARY_DIR}/KRNLDR.ELF)
add_dependencies(KernelELF Kernel terminal BootService VESATerminal SerialDriver KeyboardDriver VESADriver ATADriver FAT32Driver PMM VMM KMALLOC GDT IDT InterruptHandler PIC ZstdLib)
