# Project setup
project (Kernel_X86)

enable_language (ASM_NASM)

if(USE_DISKBUILDER)
    add_custom_target(Kernel ALL DEPENDS KRNLDR.SYS)
    add_custom_command(OUTPUT KRNLDR.SYS POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND nasm -f bin -o KRNLDR.SYS Kernel_X86.asm
        COMMENT "Compiling Core X86-64(Kernel) for diskbuilder"
        DEPENDS Kernel_X86.asm
    )
endif()



if(USE_DD)
    add_custom_target(Kernel ALL DEPENDS KRNLDR.SYS)
    add_custom_command(OUTPUT KRNLDR.SYS POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND nasm -f elf64 -o KRNLDR.SYS Kernel_X86.asm
        COMMENT "Compiling Core X86-64(Kernel)"
        DEPENDS Kernel_X86.asm
    )
endif()

