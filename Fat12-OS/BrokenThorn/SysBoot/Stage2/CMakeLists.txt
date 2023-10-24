project (boot-master)

enable_language (ASM_NASM)

add_custom_command(OUTPUT KRNLDR.SYS
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f bin -o KRNLDR.SYS Stage2.asm
    COMMENT "Compiling Core X86-64(Kernel)"
    DEPENDS Kernel_Entry_X86.asm
)
add_custom_target(Kernel ALL DEPENDS KRNLDR.SYS)