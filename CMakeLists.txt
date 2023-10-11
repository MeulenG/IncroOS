cmake_minimum_required(VERSION 3.22.1)

project("OS")

enable_language(ASM_NASM)

option(FAT32 "Build Target FAT32" ON)


set (CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
# Set linker script file path
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
set(LINKER_SCRIPT SysLink/linker.ld)
include(${CMAKE_MODULE_PATH}/CMakeDetermineRustCompiler.cmake)
include(${CMAKE_MODULE_PATH}/CMakeRustInformation.cmake)
include(${CMAKE_MODULE_PATH}/FindRust.cmake)
include(${CMAKE_MODULE_PATH}/CMakeTestRustCompiler.cmake)

set(boot-dir
    ${CMAKE_SOURCE_DIR}/SysBoot
)
set(kernel-dir
    ${CMAKE_SOURCE_DIR}/Kernel
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

add_subdirectory(Kernel)

if(FAT32)
    message(STATUS "Creating Disk Image:")
    add_custom_command(
        OUTPUT disk.img
        COMMAND cd .. && External/diskbuilder/build/osbuilder os.yaml --target img
        DEPENDS disk.img
    )

    add_custom_target(disk-image ALL DEPENDS disk.img)
endif()