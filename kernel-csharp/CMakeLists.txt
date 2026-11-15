# The kernel in C#: dotnet build turns it into IL (against Incro.CoreLib), IncroCompiler turns the IL
# into NASM, nasm makes kernel_cs.o, which boot/fat32/CMakeLists.txt links with Kernel_Entry.o instead
# of the C kernel's objects. Only added with -DINCRO_CSHARP_KERNEL=ON, see the top-level CMakeLists.txt.

get_filename_component(INCRO_COMPILER_DIR ${INCRO_COMPILER} DIRECTORY)
set(INCRO_CORELIB ${INCRO_COMPILER_DIR}/../Incro.CoreLib/Incro.CoreLib.csproj)

set(CSHARP_KERNEL_ARTIFACTS ${CMAKE_BINARY_DIR}/csharp-kernel)
set(CSHARP_KERNEL_DLL ${CSHARP_KERNEL_ARTIFACTS}/bin/IncroKernel/release/IncroKernel.dll)
set(CSHARP_KERNEL_ASM ${CMAKE_BINARY_DIR}/kernel_cs.asm)
file(GLOB_RECURSE CSHARP_KERNEL_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cs)
set(CSHARP_KERNEL_BENCH "")
if(INCRO_BENCH)
    set(CSHARP_KERNEL_BENCH -p:IncroBench=true)
endif()

add_custom_command(
    OUTPUT ${CSHARP_KERNEL_DLL}
    COMMAND dotnet build ${CMAKE_CURRENT_SOURCE_DIR}/IncroKernel.csproj -c Release --nologo -v quiet
            --artifacts-path ${CSHARP_KERNEL_ARTIFACTS} -p:IncroCoreLib=${INCRO_CORELIB} ${CSHARP_KERNEL_BENCH}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/IncroKernel.csproj ${CSHARP_KERNEL_SOURCES}
    COMMENT "Compiling the C# kernel to IL"
)

# --freestanding: no C runtime. -g: DWARF line info pointing at the C# source.
add_custom_command(
    OUTPUT ${CSHARP_KERNEL_ASM}
    COMMAND dotnet run --project ${INCRO_COMPILER} -c Release -- --freestanding -g ${CSHARP_KERNEL_DLL} ${CSHARP_KERNEL_ASM}
    DEPENDS ${CSHARP_KERNEL_DLL}
    COMMENT "Compiling the C# kernel IL to x86-64 with IncroCompiler"
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/kernel_cs.o
    COMMAND nasm -f elf64 -g -F dwarf ${CSHARP_KERNEL_ASM} -o ${CMAKE_BINARY_DIR}/kernel_cs.o
    DEPENDS ${CSHARP_KERNEL_ASM}
    COMMENT "Assembling the C# kernel"
)

add_custom_target(CSharpKernel ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel_cs.o)
