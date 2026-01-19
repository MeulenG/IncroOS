project(MM-Memory-Manager)

enable_language(C)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/pmm.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c ${CMAKE_CURRENT_SOURCE_DIR}/pmm.c -o ${CMAKE_BINARY_DIR}/pmm.o
    COMMENT "Compiling Physical Memory Manager"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pmm.c ${CMAKE_CURRENT_SOURCE_DIR}/pmm.h
)

add_custom_target(PMM ALL DEPENDS ${CMAKE_BINARY_DIR}/pmm.o)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/vmm.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c ${CMAKE_CURRENT_SOURCE_DIR}/vmm.c -o ${CMAKE_BINARY_DIR}/vmm.o
    COMMENT "Compiling Virtual Memory Manager"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vmm.c ${CMAKE_CURRENT_SOURCE_DIR}/vmm.h ${CMAKE_BINARY_DIR}/pmm.o
)

add_custom_target(VMM ALL DEPENDS ${CMAKE_BINARY_DIR}/vmm.o)
add_dependencies(VMM PMM)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/kmalloc.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c ${CMAKE_CURRENT_SOURCE_DIR}/kmalloc.c -o ${CMAKE_BINARY_DIR}/kmalloc.o
    COMMENT "Compiling Kernel Memory Allocator"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kmalloc.c ${CMAKE_CURRENT_SOURCE_DIR}/kmalloc.h ${CMAKE_BINARY_DIR}/pmm.o ${CMAKE_BINARY_DIR}/vmm.o
)

add_custom_target(KMALLOC ALL DEPENDS ${CMAKE_BINARY_DIR}/kmalloc.o)
add_dependencies(KMALLOC PMM VMM)