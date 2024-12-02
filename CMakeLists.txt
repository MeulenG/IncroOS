cmake_minimum_required(VERSION 3.22.1)

project("MFOS")

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
    

    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/KRNLDR.BIN
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND objcopy -O binary ${CMAKE_BINARY_DIR}/KRNLDR.ELF ${CMAKE_BINARY_DIR}/KRNLDR.BIN
        COMMENT "Converting ELF to BIN"
        DEPENDS ${CMAKE_BINARY_DIR}/KRNLDR.ELF
    )
    
    add_custom_target(KernelBin ALL DEPENDS KernelELF ${CMAKE_BINARY_DIR}/KRNLDR.BIN)

    add_custom_command(
        OUTPUT disk.img
        COMMAND cd .. && External/diskbuilder/build/osbuilder os.yaml --target img
        DEPENDS ${CMAKE_BINARY_DIR}/BOOT.SYS 
                ${CMAKE_BINARY_DIR}/STAGE2.SYS 
                ${CMAKE_BINARY_DIR}/KRNLDR.BIN
        COMMENT "Building Disk Image"
    )

    add_custom_target(disk-image ALL DEPENDS disk.img Boot stage2 KernelELF)

endif(MFOS)
