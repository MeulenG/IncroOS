# C# kernel code: dotnet build turns it into IL, IncroCompiler turns the IL into NASM,
# nasm makes the object that boot/fat32/CMakeLists.txt links into KRNLDR.ELF.
# Only added with -DINCRO_CSHARP=ON, see the top-level CMakeLists.txt.

set(CSHARP_ARTIFACTS ${CMAKE_BINARY_DIR}/csharp)
set(CSHARP_DLL ${CSHARP_ARTIFACTS}/bin/KernelCSharp/release/KernelCSharp.dll)
set(CSHARP_ASM ${CMAKE_BINARY_DIR}/kernel_csharp.asm)
file(GLOB CSHARP_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cs)

add_custom_command(
    OUTPUT ${CSHARP_DLL}
    COMMAND dotnet build ${CMAKE_CURRENT_SOURCE_DIR}/KernelCSharp.csproj -c Release --nologo -v quiet
            --artifacts-path ${CSHARP_ARTIFACTS}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/KernelCSharp.csproj ${CSHARP_SOURCES}
    COMMENT "Compiling kernel C# to IL"
)

# Also writes kernel_csharp.il and kernel_csharp.ir next to the .asm for inspection
add_custom_command(
    OUTPUT ${CSHARP_ASM}
    COMMAND dotnet run --project ${INCRO_COMPILER} -c Release -- ${CSHARP_DLL} ${CSHARP_ASM}
    BYPRODUCTS ${CMAKE_BINARY_DIR}/kernel_csharp.il ${CMAKE_BINARY_DIR}/kernel_csharp.ir
    DEPENDS ${CSHARP_DLL}
    COMMENT "Compiling kernel C# IL to x86-64 with IncroCompiler"
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/kernel_csharp.o
    COMMAND nasm -f elf64 ${CSHARP_ASM} -o ${CMAKE_BINARY_DIR}/kernel_csharp.o
    DEPENDS ${CSHARP_ASM}
    COMMENT "Assembling kernel C#"
)

add_custom_target(KernelCSharp ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel_csharp.o)
