project(Drivers)

message(CHECK_START "Finding CPU subsystems")
list(APPEND CMAKE_MESAGE_INDENT "  ")
unset(check_Kernel_Drivers)

message(CHECK_START "Locating CPU subsystems Directory")
# do checks, but assume we find it
message(CHECK_PASS "Found Directory")

message(CHECK_START "Making sure all systems are present")
# Check
message(CHECK_PASS "Compiling Systems....")

list(POP_BACK CMAKE_MESSAGE_INDENT)
if(check_Kernel_C)
    message(CHECK_FAIL "Missing File: ${check_Kernel_Drivers}")
else()
    message(CHECK_PASS "Found CPU subsystems")
endif()

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/gdt.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND x86_64-linux-gnu-gcc -c ${CMAKE_CURRENT_SOURCE_DIR}/gdt.c -o ${CMAKE_BINARY_DIR}/gdt.o -ffreestanding -O2 -Wall -Wextra -Werror -fno-exceptions -m64
    COMMENT "Compiling GDT"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gdt.c
)
add_custom_target(GDT ALL DEPENDS ${CMAKE_BINARY_DIR}/gdt.o)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/idt.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c ${CMAKE_CURRENT_SOURCE_DIR}/idt.c -o ${CMAKE_BINARY_DIR}/idt.o
    COMMENT "Compiling IDT"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/idt.c
)
add_custom_target(IDT ALL DEPENDS ${CMAKE_BINARY_DIR}/idt.o)