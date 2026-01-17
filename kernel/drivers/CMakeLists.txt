project(Drivers)

message(CHECK_START "Finding C Drivers")
list(APPEND CMAKE_MESAGE_INDENT "  ")
unset(check_Kernel_Drivers)

message(CHECK_START "Locating Kernel Drivers Directory")
# do checks, but assume we find it
message(CHECK_PASS "Found Directory")

message(CHECK_START "Making sure all drivers are present")
# Check
message(CHECK_PASS "Compiling Drivers....")

list(POP_BACK CMAKE_MESSAGE_INDENT)
if(check_Kernel_C)
    message(CHECK_FAIL "Missing File: ${check_Kernel_Drivers}")
else()
    message(CHECK_PASS "Found Kernel C Drivers")
endif()

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/vga.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND x86_64-linux-gnu-gcc -c ${CMAKE_CURRENT_SOURCE_DIR}/vga.c -o ${CMAKE_BINARY_DIR}/vga.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -m64
    COMMENT "Compiling VGA Driver"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vga.c
)
add_custom_target(VGADriver ALL DEPENDS ${CMAKE_BINARY_DIR}/vga.o)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/serial_asm.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f elf64 -o ${CMAKE_BINARY_DIR}/serial_asm.o ${CMAKE_CURRENT_SOURCE_DIR}/serial.asm
    COMMENT "Compiling Serial Driver (Assembly)"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/serial.asm
)
add_custom_target(SerialDriverAsm ALL DEPENDS ${CMAKE_BINARY_DIR}/serial_asm.o)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/serial.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c ${CMAKE_CURRENT_SOURCE_DIR}/serial.c -o ${CMAKE_BINARY_DIR}/serial.o
    COMMENT "Compiling Serial Driver (C)"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/serial.c
)
add_custom_target(SerialDriver ALL DEPENDS ${CMAKE_BINARY_DIR}/serial.o)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/keyboard_asm.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND nasm -f elf64 -o ${CMAKE_BINARY_DIR}/keyboard_asm.o ${CMAKE_CURRENT_SOURCE_DIR}/keyboard.asm
    COMMENT "Compiling Keyboard Driver (Assembly)"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/keyboard.asm
)
add_custom_target(KeyboardDriverAsm ALL DEPENDS ${CMAKE_BINARY_DIR}/keyboard_asm.o)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/keyboard.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c ${CMAKE_CURRENT_SOURCE_DIR}/keyboard.c -o ${CMAKE_BINARY_DIR}/keyboard.o
    COMMENT "Compiling Keyboard Driver (C)"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/keyboard.c
)
add_custom_target(KeyboardDriver ALL DEPENDS ${CMAKE_BINARY_DIR}/keyboard.o)