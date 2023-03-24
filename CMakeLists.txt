cmake_minimum_required(VERSION 3.22.1)

project("OS")

enable_language(ASM_NASM)

set (CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set (ASM_BIN
    SysBoot/STAGE1.SYS
    SysBoot/STAGE2.SYS
    SysCore/KRNLDR.SYS
)

set(C_SOURCES
    SysKernel/kernel.c
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

add_subdirectory(SysKernel)

add_executable(${PROJECT_NAME} ${C_SOURCES} ${ASM_BIN})

message(STATUS "Linking Libraries With Project")
add_custom_command(
    OUTPUT disk.img
    COMMAND cd .. && External/diskbuilder/build/osbuilder os.yaml --target img
    DEPENDS ${PROJECT_NAME}
)

add_custom_target(disk-image ALL DEPENDS disk.img)


TARGET_LINK_LIBRARIES(${PROJECT_NAME})