# Project setup
project (kernel)

enable_language (ASM_NASM)


add_custom_target(Kernel_X86 ALL DEPENDS KRNLDR.sys)
add_custom_command(OUTPUT KRNLDR.sys POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Kernel_X86
    COMMAND nasm ARGS -f bin -o KRNLDR.sys Kernel_X86.asm
    COMMENT "Compiling Core (Kernel)"
    DEPENDS SysCore/Kernel_X86.asm
)