project (boot-master)

enable_language (ASM_NASM)

add_custom_command(OUTPUT Boot1.bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f bin -o Boot1.bin Boot1.asm
    COMMENT "Compiling bootloader (Boot1)"
    DEPENDS Boot1.asm
)
add_custom_target(Boot ALL DEPENDS Boot1.bin)