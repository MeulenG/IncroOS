project(Libs)

message(CHECK_START "Finding C Libs")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
unset(check_Kernel_Libs)

message(CHECK_START "Locating Kernel Libraries Directory")
# do checks, but assume we find it
message(CHECK_PASS "Found Directory")

message(CHECK_START "Making sure all libs are present")
# Check
message(CHECK_PASS "Compiling Libs....")

list(POP_BACK CMAKE_MESSAGE_INDENT)
if(check_Kernel_Libs)
    message(CHECK_FAIL "Missing File: ${check_Kernel_Libs}")
else()
    message(CHECK_PASS "Found Kernel C Libs")
endif()

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/print.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -c ${CMAKE_CURRENT_SOURCE_DIR}/print.c -o ${CMAKE_BINARY_DIR}/print.o -ffreestanding -O2 -Wall -Wextra -Werror -fno-exceptions -m64
    COMMENT "Compiling Print Lib"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/print.c
)
add_custom_target(PrintLib ALL DEPENDS ${CMAKE_BINARY_DIR}/print.o)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/string.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -c ${CMAKE_CURRENT_SOURCE_DIR}/string.c -o ${CMAKE_BINARY_DIR}/string.o -ffreestanding -O2 -Wall -Wextra -Werror -fno-exceptions -m64
    COMMENT "Compiling String Lib"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/string.c
)
add_custom_target(StringLib ALL DEPENDS ${CMAKE_BINARY_DIR}/string.o)
