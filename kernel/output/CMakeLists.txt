project(Kernel-Output)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/terminal.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c ${CMAKE_CURRENT_SOURCE_DIR}/terminal.c -o ${CMAKE_BINARY_DIR}/terminal.o
    COMMENT "Compiling terminal.c which depends on kernel.c"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/terminal.c
)

add_custom_target(terminal ALL DEPENDS ${CMAKE_BINARY_DIR}/terminal.o)