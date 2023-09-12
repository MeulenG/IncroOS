project (boot-master)

enable_language (ASM_NASM)


add_custom_command(OUTPUT STAGE1.SYS
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f bin -o STAGE1.SYS Stage1_X86.asm
    COMMENT "Compiling bootloader (stage1)"
    DEPENDS Stage1_X86.asm
)
add_custom_target(stage1 ALL DEPENDS STAGE1.SYS)

add_custom_command(OUTPUT STAGE2.SYS
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f bin -o STAGE2.SYS Stage2_X86.asm
    COMMENT "Compiling bootloader (stage2)"
    DEPENDS Stage2_X86.asm
)
add_custom_target(stage2 ALL DEPENDS STAGE2.SYS)

add_custom_command(OUTPUT KRNLDR.SYS
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f bin -o KRNLDR.SYS Kernel_Entry_X86.asm
    COMMENT "Compiling Core X86-64(Kernel)"
    DEPENDS Kernel_Entry_X86.asm
)
add_custom_target(Kernel ALL DEPENDS KRNLDR.SYS)