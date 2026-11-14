# C# kernel code: dotnet build turns it into IL (against Incro.CoreLib), IncroCompiler turns the IL
# into NASM, nasm makes the object that boot/fat32/CMakeLists.txt links into KRNLDR.ELF.
# Only added with -DINCRO_CSHARP=ON, see the top-level CMakeLists.txt.

get_filename_component(INCRO_COMPILER_DIR ${INCRO_COMPILER} DIRECTORY)
set(INCRO_CORELIB ${INCRO_COMPILER_DIR}/../Incro.CoreLib/Incro.CoreLib.csproj)

set(CSHARP_ARTIFACTS ${CMAKE_BINARY_DIR}/csharp)
set(CSHARP_DLL ${CSHARP_ARTIFACTS}/bin/KernelCSharp/release/KernelCSharp.dll)
set(CSHARP_ASM ${CMAKE_BINARY_DIR}/kernel_csharp.asm)
file(GLOB CSHARP_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cs)

add_custom_command(
    OUTPUT ${CSHARP_DLL}
    COMMAND dotnet build ${CMAKE_CURRENT_SOURCE_DIR}/KernelCSharp.csproj -c Release --nologo -v quiet
            --artifacts-path ${CSHARP_ARTIFACTS} -p:IncroCoreLib=${INCRO_CORELIB}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/KernelCSharp.csproj ${CSHARP_SOURCES}
    COMMENT "Compiling kernel C# to IL"
)

# --freestanding: no C runtime (no C main, nothing from libc). -g: %line directives, so the object's
# DWARF line info points at the C# source. Also writes kernel_csharp.il and .ir next to the .asm.
add_custom_command(
    OUTPUT ${CSHARP_ASM}
    COMMAND dotnet run --project ${INCRO_COMPILER} -c Release -- --freestanding -g ${CSHARP_DLL} ${CSHARP_ASM}
    BYPRODUCTS ${CMAKE_BINARY_DIR}/kernel_csharp.il ${CMAKE_BINARY_DIR}/kernel_csharp.ir
    DEPENDS ${CSHARP_DLL}
    COMMENT "Compiling kernel C# IL to x86-64 with IncroCompiler"
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/kernel_csharp.o
    COMMAND nasm -f elf64 -g -F dwarf ${CSHARP_ASM} -o ${CMAKE_BINARY_DIR}/kernel_csharp.o
    DEPENDS ${CSHARP_ASM}
    COMMENT "Assembling kernel C#"
)

# incro_alloc and incro_panic, the kernel side of the C# runtime
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/csharp_runtime.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -march=x86-64 -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -Wall -Wextra -c ${CMAKE_CURRENT_SOURCE_DIR}/runtime.c -o ${CMAKE_BINARY_DIR}/csharp_runtime.o
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/runtime.c
    COMMENT "Compiling the C# runtime hooks (C)"
)

add_custom_target(KernelCSharp ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel_csharp.o ${CMAKE_BINARY_DIR}/csharp_runtime.o)
