project(Kernel-Output)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/terminal.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -Werror -c ${CMAKE_CURRENT_SOURCE_DIR}/vga_terminal.c -o ${CMAKE_BINARY_DIR}/terminal.o
    COMMENT "Compiling terminal.c which depends on kernel.c"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vga_terminal.c
)

add_custom_target(terminal ALL DEPENDS ${CMAKE_BINARY_DIR}/terminal.o)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/boot_service.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -Werror -c ${CMAKE_CURRENT_SOURCE_DIR}/boot_service.c -o ${CMAKE_BINARY_DIR}/boot_service.o
    COMMENT "Compiling boot_service.c"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/boot_service.c
)

add_custom_target(BootService ALL DEPENDS ${CMAKE_BINARY_DIR}/boot_service.o)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/vesa_terminal.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -Werror -c ${CMAKE_CURRENT_SOURCE_DIR}/vesa_terminal.c -o ${CMAKE_BINARY_DIR}/vesa_terminal.o
    COMMENT "Compiling vesa_terminal.c"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vesa_terminal.c
)

add_custom_target(VESATerminal ALL DEPENDS ${CMAKE_BINARY_DIR}/vesa_terminal.o)