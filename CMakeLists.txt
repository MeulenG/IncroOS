cmake_minimum_required(VERSION 3.22.1)

project("OS")

enable_language(ASM_NASM)

# Define the build options
option(USE_DISKBUILDER "Build Target DISKBUILDER" OFF)
option(USE_DD "Build Target DD" ON)
option(STABLE "Build Target STABLE" OFF)


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
    SysBoot
)
set(core-dir
    SysCore
)

message(CHECK_START "Attempting to find system bootloaders")
if(EXISTS "${boot-dir}" AND IS_DIRECTORY "${boot-dir}")
    message("Found System BootLoaders")
else()
    message(CHECK_FAIL "${boot-dir} doesn't exist")
endif()
add_subdirectory(SysBoot)

message(CHECK_START "Attempting to find system core")
if(EXISTS "${core-dir}" AND IS_DIRECTORY "${core-dir}")
    message("Found System Core")
else()
    message(CHECK_FAIL "${core-dir} doesn't exist")
endif()

add_subdirectory(SysCore)

add_subdirectory(SysKernel)# Depending on the build options, build the targets from the source directory
if(USE_DISKBUILDER)
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

if(USE_DD)
    add_custom_command(
        OUTPUT Incro-OS
        COMMAND ld -nostdlib -T SysKernel/arch/amd64/link.ld -o Incro-OS SysCore/KRNLDR.SYS /home/oliverm/Desktop/Programming/Incro-OS/SysKernel/Incro-Kernel/target/debug/deps/Incro_Kernel-4d6ddc1619e61400.1nb7gasd51bhk1x.rcgu.o
        DEPENDS SysCore/KRNLDR.SYS SysKernel/Incro-Kernel/target/debug/Incro-Kernel
    )

    message(STATUS "Attempting to build image")
    add_custom_command(
        OUTPUT Incro-OS.bin
        COMMAND objcopy -O binary Incro-OS Incro-OS.bin
        DEPENDS Incro-OS
    )


    message(STATUS "Linking Libraries With Project")
    add_custom_command(
        OUTPUT disk.img
        COMMAND dd if=SysBoot/STAGE1.SYS of=disk.img bs=512 count=1 conv=notrunc && dd if=SysBoot/Stage2.SYS of=disk.img bs=512 count=5 seek=1 conv=notrunc && dd if=Incro-OS.bin of=disk.img bs=512 count=100 seek=6 conv=notrunc
        DEPENDS Incro-OS.bin
    )

    add_custom_target(Incro-image ALL DEPENDS disk.img)
    add_custom_target(Incro-OS ALL DEPENDS Incro-OS.bin)
    add_custom_target(disk-image ALL DEPENDS disk.img)
endif()

if(STABLE)
    message(STATUS "Linking Libraries With Project")
    add_custom_command(
        OUTPUT disk.img
        COMMAND cd .. && External/diskbuilder/build/osbuilder os.yaml --target img
        DEPENDS disk.img
    )
    # Add custom command to link kernel object file with linker script
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel
        COMMAND ld -nostdlib -o kernel kernel.o main.o
        DEPENDS kernel
    )

    add_custom_target(disk-image ALL DEPENDS disk.img)
endif()