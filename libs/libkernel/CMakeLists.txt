project(LibKernel)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/print.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -c ${CMAKE_CURRENT_SOURCE_DIR}/print.c -o ${CMAKE_BINARY_DIR}/print.o
            -I${CMAKE_SOURCE_DIR}/kernel
            -ffreestanding -O2 -Wall -Wextra -Werror -fno-exceptions -m64
    COMMENT "Compiling Print Lib"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/print.c
)
add_custom_target(PrintLib ALL DEPENDS ${CMAKE_BINARY_DIR}/print.o)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/string.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -c ${CMAKE_CURRENT_SOURCE_DIR}/string.c -o ${CMAKE_BINARY_DIR}/string.o
            -ffreestanding -O2 -Wall -Wextra -Werror -fno-exceptions -m64
    COMMENT "Compiling String Lib"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/string.c
)
add_custom_target(StringLib ALL DEPENDS ${CMAKE_BINARY_DIR}/string.o)
