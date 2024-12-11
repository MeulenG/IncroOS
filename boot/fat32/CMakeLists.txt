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
    COMMAND ld -o ${CMAKE_BINARY_DIR}/KRNLDR.ELF ${CMAKE_BINARY_DIR}/Kernel_Entry.o ${CMAKE_BINARY_DIR}/kernel.o ${CMAKE_BINARY_DIR}/terminal.o -Ttext=0x500 --entry=_start
    COMMENT "Linking Kernel to ELF"
    DEPENDS ${CMAKE_BINARY_DIR}/Kernel_Entry.o ${CMAKE_BINARY_DIR}/kernel.o ${CMAKE_BINARY_DIR}/terminal.o
)

add_custom_target(KernelELF ALL DEPENDS ${CMAKE_BINARY_DIR}/KRNLDR.ELF)

add_dependencies(KernelELF Kernel terminal)