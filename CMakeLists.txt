cmake_minimum_required(VERSION 3.22.1)

project(assembler C ASM)

set (CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set (ASM_SOURCES
    SysBoot/Stage1_X86.asm
    SysBoot/Stage2_X86.asm
    SysCore/Kernel_X86.asm
)

set(C_SOURCES
    SysKernel/kernel.c
)

add_subdirectory(SysBoot)

add_subdirectory(SysCore)

add_subdirectory(SysKernel)

add_executable(OMOS)