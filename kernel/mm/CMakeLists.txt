project(Memory-Manager)

enable_language(C)

message(CHECK_START "Building Memory Manager")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

message(CHECK_START "Compiling PMM")
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/pmm.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c ${CMAKE_CURRENT_SOURCE_DIR}/pmm.c -o ${CMAKE_BINARY_DIR}/pmm.o
    COMMENT "Compiling Physical Memory Manager"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pmm.c ${CMAKE_CURRENT_SOURCE_DIR}/pmm.h
)
add_custom_target(pmm ALL DEPENDS ${CMAKE_BINARY_DIR}/pmm.o)
message(CHECK_PASS "PMM compiled")

message(CHECK_START "Compiling VMM")
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/vmm.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c ${CMAKE_CURRENT_SOURCE_DIR}/vmm.c -o ${CMAKE_BINARY_DIR}/vmm.o
    COMMENT "Compiling Virtual Memory Manager"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vmm.c ${CMAKE_CURRENT_SOURCE_DIR}/vmm.h ${CMAKE_BINARY_DIR}/pmm.o
)
add_custom_target(vmm ALL DEPENDS ${CMAKE_BINARY_DIR}/vmm.o)
add_dependencies(vmm pmm)
message(CHECK_PASS "VMM compiled")

message(CHECK_START "Compiling Heap")
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/heap.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c ${CMAKE_CURRENT_SOURCE_DIR}/heap.c -o ${CMAKE_BINARY_DIR}/heap.o
    COMMENT "Compiling Heap Allocator"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/heap.c ${CMAKE_CURRENT_SOURCE_DIR}/heap.h ${CMAKE_BINARY_DIR}/vmm.o ${CMAKE_BINARY_DIR}/pmm.o
)
add_custom_target(heap ALL DEPENDS ${CMAKE_BINARY_DIR}/heap.o)
add_dependencies(heap vmm pmm)
message(CHECK_PASS "Heap compiled")

list(POP_BACK CMAKE_MESSAGE_INDENT)
message(CHECK_PASS "Memory Manager built")
