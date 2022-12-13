# Project setup
project (Kernel_X86)

enable_language (ASM_NASM)

add_custom_target(Kernel ALL DEPENDS KRNLDR.SYS)
add_custom_command(OUTPUT KRNLDR.SYS POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f bin -o KRNLDR.SYS Kernel_X86.asm
    COMMENT "Compiling Core X86-64(Kernel)"
    DEPENDS Kernel_X86.asm
)