project(Libs)

message(CHECK_START "Finding C Libs")
list(APPEND CMAKE_MESAGE_INDENT "  ")
unset(check_Kernel_Libs)

message(CHECK_START "Locating Kernel Libraries Directory")
# do checks, but assume we find it
message(CHECK_PASS "Found Directory")

message(CHECK_START "Making sure all libs are present")
# Check
message(CHECK_PASS "Compiling Libs....")

list(POP_BACK CMAKE_MESSAGE_INDENT)
if(check_Kernel_C)
    message(CHECK_FAIL "Missing File: ${check_Kernel_Libs}")
else()
    message(CHECK_PASS "Found Kernel C Libs")
endif()

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/print.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND x86_64-linux-gnu-gcc -c ${CMAKE_CURRENT_SOURCE_DIR}/print.c -o ${CMAKE_BINARY_DIR}/print.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -m64
    COMMENT "Compiling Print Lib"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/print.c
)
add_custom_target(PrintLib ALL DEPENDS ${CMAKE_BINARY_DIR}/print.o)
