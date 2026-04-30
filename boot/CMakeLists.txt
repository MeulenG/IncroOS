# Project setup
project (boot-bios)

enable_language (ASM_NASM)

add_subdirectory(fat32)


#add_custom_target(mbr ALL DEPENDS MBR.BIN)
#add_custom_command(OUTPUT mbr.sys POST_BUILD
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#    COMMAND nasm -f bin -o ${CMAKE_BINARY_DIR}/MBR.BIN mbr.asm
#    COMMENT "Compiling Master Boot Record"
#    DEPENDS mbr.asm
#)