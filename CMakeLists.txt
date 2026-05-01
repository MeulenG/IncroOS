cmake_minimum_required(VERSION 3.22.1)

project("IncroOS")

enable_language(ASM_NASM)

option(MFOS "Building Target IncroOS" ON)
option(Barebones "Building Barebones" OFF)
set(BAKE_DIR ${CMAKE_SOURCE_DIR}/External/bake)

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

    option(AUTO_FETCH_SUBMODULES "Fetch submodules if missing" ON)

    if(NOT EXISTS ${BAKE_DIR}/CMakeLists.txt)
        if(AUTO_FETCH_SUBMODULES)
            message(STATUS "Submodule missing, fetching...")

            find_package(Git REQUIRED)

            execute_process(
                COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive External/bake
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE result
            )

            if(NOT result EQUAL 0)
                message(FATAL_ERROR "Failed to fetch submodule")
            endif()
        else()
            message(FATAL_ERROR
                "Submodule 'bake' is missing.\n"
                "Run: git submodule update --init --recursive"
            )
        endif()
    endif()
    #add_subdirectory(${BAKE_DIR})

    add_custom_command(
        OUTPUT disk.img
        COMMAND cd .. && External/bake/build/bin/mkcdk -s 512M builder.yaml
        DEPENDS ${CMAKE_BINARY_DIR}/BOOT.SYS 
                ${CMAKE_BINARY_DIR}/STAGE2.SYS 
                ${CMAKE_BINARY_DIR}/KRNLDR.BIN
        COMMENT "Building Disk Image"
    )

    add_custom_target(disk-image ALL DEPENDS disk.img Boot stage2 KernelELF)

endif(MFOS)
