cmake_minimum_required(VERSION 3.22.1)

project("OS")

enable_language(ASM_NASM)

set (CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set (ASM_BIN
    SysBoot/stage1.sys
    SysBoot/stage2.sys
    SysCore/KRNLDR.sys
)

set(C_SOURCES
    SysKernel/kernel.c
)

add_subdirectory(SysBoot)

add_subdirectory(SysCore)

add_subdirectory(SysKernel)

add_executable(${PROJECT_NAME} ${C_SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME})