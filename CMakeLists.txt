cmake_minimum_required(VERSION 3.22.1)

project("IncroOS")

enable_language(ASM_NASM)

option(MFOS "Building Target IncroOS" ON)
option(Barebones "Building Barebones" OFF)
set(BAKE_DIR ${CMAKE_SOURCE_DIR}/External/bake)

# Kernel code written in C# (kernel/csharp), compiled to x86-64 by IncroCompiler and linked into the kernel.
# Off by default: it needs the .NET 8 SDK and an IncroCompiler checkout.
option(INCRO_CSHARP "Link the C# code in kernel/csharp into the kernel" OFF)
# Or the whole kernel in C#: kernel-csharp, a port of kernel/, linked instead of it
option(INCRO_CSHARP_KERNEL "Build the kernel from kernel-csharp (C#) instead of kernel (C)" OFF)
# Benchmarks that either kernel runs after its boot services (kernel/bench, kernel-csharp/Bench.cs)
option(INCRO_BENCH "Run the kernel benchmarks at boot" OFF)
set(INCRO_COMPILER ${CMAKE_SOURCE_DIR}/../IncroCompiler/IncroCompiler/IncroCompiler.csproj
    CACHE FILEPATH "IncroCompiler.csproj used when INCRO_CSHARP or INCRO_CSHARP_KERNEL is ON")

set (CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
# Let's check for dependencies
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

if(MFOS)
    set(boot-dir ${CMAKE_SOURCE_DIR}/boot)
    set(kernel-dir ${CMAKE_SOURCE_DIR}/kernel)

    # boot/ links the kernel and is configured before kernel/, so the object list is decided here
    set(CSHARP_OBJECTS "")
    if(INCRO_CSHARP AND INCRO_CSHARP_KERNEL)
        message(FATAL_ERROR "INCRO_CSHARP adds C# to the C kernel and INCRO_CSHARP_KERNEL replaces it; pick one")
    endif()
    if((INCRO_CSHARP OR INCRO_CSHARP_KERNEL) AND NOT EXISTS ${INCRO_COMPILER})
        message(FATAL_ERROR "C# is ON but ${INCRO_COMPILER} does not exist, "
                            "pass -DINCRO_COMPILER=<path to IncroCompiler.csproj>")
    endif()
    if(INCRO_CSHARP)
        set(CSHARP_OBJECTS ${CMAKE_BINARY_DIR}/kernel_csharp.o ${CMAKE_BINARY_DIR}/csharp_runtime.o)
    endif()

    add_subdirectory(libs)
    add_subdirectory(boot)
    if(INCRO_CSHARP_KERNEL)
        add_subdirectory(kernel-csharp)
    else()
        add_subdirectory(kernel)
    endif()
    

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

    # defaults to the bake submodule build (as in CI), override with -DMKCDK=<path> for a local bake checkout
    set(MKCDK ${BAKE_DIR}/build/bin/mkcdk CACHE FILEPATH "Path to the mkcdk disk image tool")

    add_custom_command(
        OUTPUT disk.img
        COMMAND cd .. && ${MKCDK} -s 2G builder.yaml
        DEPENDS ${CMAKE_BINARY_DIR}/BOOT.SYS 
                ${CMAKE_BINARY_DIR}/STAGE2.SYS 
                ${CMAKE_BINARY_DIR}/KRNLDR.BIN
        COMMENT "Building Disk Image"
    )

    add_custom_target(disk-image ALL DEPENDS disk.img Boot stage2 KernelELF)

endif(MFOS)
