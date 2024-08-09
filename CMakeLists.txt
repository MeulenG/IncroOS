cmake_minimum_required(VERSION 3.22.1)

project("OS")

enable_language(ASM_NASM)

option(MFOS "Building Target MFOS" ON)
option(Barebones "Building Barebones" OFF)

set (CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
# Let's check for dependencies
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

if(MFOS)
    set(boot-dir ${CMAKE_SOURCE_DIR}/boot)
    set(kernel-dir ${CMAKE_SOURCE_DIR}/kernel)

    add_subdirectory(boot)
    add_subdirectory(kernel)

    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/kernel.bin
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND x86_64-linux-gnu-ld -T ${CMAKE_SOURCE_DIR}/linker.ld -o ${CMAKE_BINARY_DIR}/kernel.bin ${CMAKE_BINARY_DIR}/vga.o ${CMAKE_BINARY_DIR}/kernel.o ${CMAKE_BINARY_DIR}/entry.o
        DEPENDS ${CMAKE_BINARY_DIR}/kernel.o ${CMAKE_BINARY_DIR}/entry.o
    )
    add_custom_target(MyOS ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel.bin)

    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/kernel.raw
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND x86_64-linux-gnu-objcopy -O binary ${CMAKE_BINARY_DIR}/kernel.bin ${CMAKE_BINARY_DIR}/kernel.raw
        DEPENDS ${CMAKE_BINARY_DIR}/kernel.bin
    )
    add_custom_target(KernelBin ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel.raw)

    add_custom_command(
        OUTPUT disk.img
        COMMAND cd .. && External/diskbuilder/build/osbuilder os.yaml --target img
        DEPENDS ${CMAKE_BINARY_DIR}/BOOT.SYS ${CMAKE_BINARY_DIR}/STAGE2.SYS ${CMAKE_BINARY_DIR}/kernel.bin ${CMAKE_BINARY_DIR}/kernel.raw
    )
    add_custom_target(disk-image ALL DEPENDS disk.img)

    # Explicit dependencies
    add_dependencies(MyOS VGADriver Kernel KernelEntry Boot stage2)
    add_dependencies(KernelBin MyOS)
    add_dependencies(disk-image MyOS KernelBin)
endif(MFOS)
