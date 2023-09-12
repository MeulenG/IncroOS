cmake_minimum_required(VERSION 3.22.1)

project("OS")

enable_language(ASM_NASM)

# Define the build options
option(FAT12 "Build Target DISKBUILDER" OFF)
option(FAT16 "Build Target DD" OFF)
option(FAT32 "Build Target STABLE" ON)


set (CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
# Set linker script file path
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
set(LINKER_SCRIPT SysLink/linker.ld)
include(${CMAKE_MODULE_PATH}/CMakeDetermineRustCompiler.cmake)
include(${CMAKE_MODULE_PATH}/CMakeRustInformation.cmake)
include(${CMAKE_MODULE_PATH}/FindRust.cmake)
include(${CMAKE_MODULE_PATH}/CMakeTestRustCompiler.cmake)

# Add the Rust source file to the project
set(RUST_SOURCES
    SysKernel/kernel.rs
)

set (ASM_BIN
    SysBoot/STAGE1.SYS
    SysBoot/STAGE2.SYS
    SysCore/KRNLDR.SYS
)

set(C_SOURCES
    SysKernel/kernel.rs
)

set(boot-dir
    ${CMAKE_SOURCE_DIR}/SysBoot
)
set(kernel-dir
    ${CMAKE_SOURCE_DIR}/SysKernel
)

message(CHECK_START "Attempting to find system bootloaders")
if(EXISTS "${boot-dir}" AND IS_DIRECTORY "${boot-dir}")
    message("Found System BootLoaders")
else()
    message(CHECK_FAIL "${boot-dir} doesn't exist")
endif()
add_subdirectory(SysBoot)

message(CHECK_START "Attempting to find kernel")
if(EXISTS "${kernel-dir}" AND IS_DIRECTORY "${kernel-dir}")
    message("Found Kernel")
else()
    message(CHECK_FAIL "${kernel-dir} doesn't exist")
endif()

add_subdirectory(SysKernel)

if(FAT12)
    message(STATUS "Attempting to link object files")
    add_custom_command(
        OUTPUT KRN
        COMMAND ld -nostdlib -T link.ld -o KRN kernel.o main.o
        DEPENDS KRN
    )

    message(STATUS "Attempting to build image")
    add_custom_command(
        OUTPUT KRNLDR.BIN
        COMMAND binary KRN KRNLDR.bin
        DEPENDS KRNLDR.BIN
    )

    message(STATUS "Linking Libraries With Project")
    add_custom_command(
        OUTPUT disk.img
        COMMAND cd .. && External/diskbuilder/build/osbuilder os.yaml --target img
        DEPENDS disk.img
    )

    add_custom_target(KRN ALL DEPENDS KRN)
    add_custom_target(KRNLDR.bin ALL DEPENDS KRNLDR.bin)
    add_custom_target(disk-image ALL DEPENDS disk.img)
endif()

if(FAT16)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Incro-OS
        COMMAND ld -nostdlib -T SysKernel/Kernel-Rust/arch/amd64/link.ld -o ${CMAKE_CURRENT_BINARY_DIR}/Incro-OS SysKernel/Kernel-Rust/target/release/deps/libkernel-b70872a646538ec6.a
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/SysCore/KRNLDR.SYS ${CMAKE_CURRENT_SOURCE_DIR}/SysKernel/Incro-Kernel/target/debug/Incro-Kernel
    )

    message(STATUS "Attempting to build image")
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Incro-OS.bin
        COMMAND objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/Incro-OS ${CMAKE_CURRENT_BINARY_DIR}/Incro-OS.bin
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Incro-OS
    )

    message(STATUS "Linking Libraries With Project")
    add_custom_command(
        OUTPUT disk.img
        COMMAND dd if=SysBoot/STAGE1.SYS of=disk.img bs=512 count=1 conv=notrunc && dd if=SysBoot/STAGE2.SYS of=disk.img bs=512 count=5 seek=1 conv=notrunc && dd if=Incro-OS.bin of=disk.img bs=512 count=100 seek=6 conv=notrunc
        DEPENDS Incro-OS.bin
    )

    add_custom_target(Incro-image ALL DEPENDS disk.img)
    add_custom_target(Incro-OS ALL DEPENDS Incro-OS.bin)
    add_custom_target(disk-image ALL DEPENDS disk.img)
endif()

if(FAT32)
    message(STATUS "Linking Libraries With Project")
    add_custom_command(
        OUTPUT disk.img
        COMMAND cd .. && External/diskbuilder/build/osbuilder os.yaml --target img
        DEPENDS disk.img
    )

    add_custom_target(disk-image ALL DEPENDS disk.img)
endif()