# Project setup
project (kernel)

enable_language (ASM_NASM)

add_custom_target(Kernel ALL DEPENDS KRNLDR.sys)
add_custom_command(OUTPUT KRNLDR.sys POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f bin -o KRNLDR.sys Kernel_X86.asm
    COMMENT "Compiling Core (Kernel)"
    DEPENDS Kernel_X86.asm
)