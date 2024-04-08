project(Kernel-C)

message(CHECK_START "Finding C Kernel In Project")
list(APPEND CMAKE_MESAGE_INDENT "  ")
unset(check_Kernel_C)

message(CHECK_START "Finding System Kernel Directory")
# do checks, but assume we find it
message(CHECK_PASS "Found Directory")

message(CHECK_START "Finding C Kernel In Directory")
# Check
message(CHECK_PASS "Found Kernel C File in Directory")

list(POP_BACK CMAKE_MESSAGE_INDENT)
if(check_Kernel_C)
    message(CHECK_FAIL "Missing File: ${check_Kernel_C}")
else()
    message(CHECK_PASS "Found C Kernel")
endif()

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/kernel.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND x86_64-linux-gnu-gcc -c ${CMAKE_CURRENT_SOURCE_DIR}/kernel.c -o ${CMAKE_BINARY_DIR}/kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -m64
    COMMENT "Compiling C Kernel"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kernel.c
)
add_custom_target(Kernel ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel.o)