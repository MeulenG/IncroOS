project(FSharp-Kernel)

message(CHECK_START "Finding F# Kernel In Project")
list(APPEND CMAKE_MESAGE_INDENT "  ")
unset(check_Kernel_C)

message(CHECK_START "Finding System Kernel Directory")
# do checks, but assume we find it
message(CHECK_PASS "Found Directory")

message(CHECK_START "Finding F# Kernel In Directory")
# Check
message(CHECK_PASS "Found F# Kernel File in Directory")

list(POP_BACK CMAKE_MESSAGE_INDENT)
if(check_Kernel_C)
    message(CHECK_FAIL "Missing File: ${check_Kernel_C}")
else()
    message(CHECK_PASS "Found F# Kernel")
endif()

add_library(kernel Libary.fs)