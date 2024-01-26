project (mfos-output)
enable_language (C)

add_custom_command(OUTPUT kernel.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND i686-elf-gcc -c terminal.c -o terminal.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions
    COMMENT "Compiling C Kernel"
    DEPENDS terminal.c
)
add_custom_target(Terminal ALL DEPENDS terminal.o)