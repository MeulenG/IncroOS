# Project setup
project (boot-bios)

enable_language (ASM_NASM)

add_custom_target(stage1 ALL DEPENDS stage1.sys)
add_custom_command(OUTPUT stage1.sys POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f bin -o stage1.sys Stage1_X86.asm
    COMMENT "Compiling bootloader (stage1)"
    DEPENDS Stage1_X86.asm
)

add_custom_target(stage2 ALL DEPENDS stage2.sys)
add_custom_command(OUTPUT stage2.sys POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f bin -o Stage2.sys Stage2_X86.asm
    COMMENT "Compiling bootloader (stage2)"
    DEPENDS Stage2_X86.asm
)