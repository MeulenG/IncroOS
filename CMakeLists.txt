cmake_minimum_required(VERSION 3.22.1)

project("OS")

enable_language(ASM_NASM)

option(FAT32 "Build Target FAT32" OFF)
option(FAT12 "Build Target FAT12" ON)


set (CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
# Set linker script file path
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
include(${CMAKE_MODULE_PATH}/CMakeDetermineRustCompiler.cmake)
include(${CMAKE_MODULE_PATH}/CMakeRustInformation.cmake)
include(${CMAKE_MODULE_PATH}/FindRust.cmake)
include(${CMAKE_MODULE_PATH}/CMakeTestRustCompiler.cmake)

if(FAT32)
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
    message(STATUS "Creating Disk Image:")
    add_custom_command(
        OUTPUT disk.img
        COMMAND cd .. && External/diskbuilder/build/osbuilder os.yaml --target img
        DEPENDS disk.img
    )

    add_custom_target(disk-image ALL DEPENDS disk.img)
endif()

if(FAT12)
    set(fat-12-root-dir
    ${CMAKE_SOURCE_DIR}/Fat12-OS
    )
    message(CHECK_START "Attempting to find kernel")
    if(EXISTS "${fat-12-root-dir}" AND IS_DIRECTORY "${fat-12-root-dir}")
        message("Found Kernel")
    else()
        message(CHECK_FAIL "${fat-12-root-dir} doesn't exist")
    endif()

    set(fat-12-boot-dir
    ${CMAKE_SOURCE_DIR}/Fat12-OS/BrokenThorn/SysBoot
    )

    message(CHECK_START "Attempting to find system bootloaders")
    if(EXISTS "${fat-12-boot-dir}" AND IS_DIRECTORY "${fat-12-boot-dir}")
        message("Found Fat-12 System BootLoaders")
    else()
        message(CHECK_FAIL "${fat-12-boot-dir} doesn't exist")
    endif()

    execute_process(
        COMMAND sudo sh -c "echo :WindowsBatch:E::bat::/init: > /proc/sys/fs/binfmt_misc/register"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )
    execute_process(
        COMMAND ${CMAKE_SOURCE_DIR}/Fat12-OS/Setup/Floppy.bat
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )
    
    
    if(result)
        message("ERROR: ${error}")
    else()
        message("OUTPUT: ${output}")
    endif()    
endif(FAT12)
