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