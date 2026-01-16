project(Kernel-Interrupts)

# Compile ISR assembly
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/isr.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f elf64 -o ${CMAKE_BINARY_DIR}/isr.o ${CMAKE_CURRENT_SOURCE_DIR}/isr.asm
    COMMENT "Compiling ISR Assembly"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/isr.asm
)
add_custom_target(ISRAsm ALL DEPENDS ${CMAKE_BINARY_DIR}/isr.o)

# Compile interrupts.c
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/interrupts.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c ${CMAKE_CURRENT_SOURCE_DIR}/interrupts.c -o ${CMAKE_BINARY_DIR}/interrupts.o
    COMMENT "Compiling interrupts.c"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/interrupts.c
)
add_custom_target(Interrupts ALL DEPENDS ${CMAKE_BINARY_DIR}/interrupts.o)

# Compile pic.c
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/pic.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c ${CMAKE_CURRENT_SOURCE_DIR}/pic.c -o ${CMAKE_BINARY_DIR}/pic.o
    COMMENT "Compiling pic.c"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pic.c
)
add_custom_target(PIC ALL DEPENDS ${CMAKE_BINARY_DIR}/pic.o)

# Compile pit.c
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/pit.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c ${CMAKE_CURRENT_SOURCE_DIR}/pit.c -o ${CMAKE_BINARY_DIR}/pit.o
    COMMENT "Compiling pit.c"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pit.c
)
add_custom_target(PIT ALL DEPENDS ${CMAKE_BINARY_DIR}/pit.o)
