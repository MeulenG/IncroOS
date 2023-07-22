cmake_minimum_required(VERSION 3.20)
project(kernel)

set(ARCH "amd64" CACHE STRING "Architecture to build for")
set(TRIPLE "" CACHE STRING "Target triple for the toolchain")

if (ARCH STREQUAL "amd64")
    set(TRIPLE "x86_64-none-elf-" CACHE STRING "Target triple for the toolchain")
elseif (ARCH STREQUAL "x86")
    set(TRIPLE "i686-elf-" CACHE STRING "Target triple for the toolchain")
else()
    message(FATAL_ERROR "Unknown architecture ${ARCH}")
endif()

set(CARGO "cargo" CACHE STRING "Path to the cargo tool")
set(RUSTC "rustc" CACHE STRING "Path to the rustc tool")
set(LD "${TRIPLE}ld" CACHE STRING "Path to the linker tool")
set(AS "${TRIPLE}as" CACHE STRING "Path to the assembler tool")
set(OBJDUMP "${TRIPLE}objdump" CACHE STRING "Path to the objdump tool")
set(OBJCOPY "${TRIPLE}objcopy" CACHE STRING "Path to the objcopy tool")

set(OBJDIR "${CMAKE_CURRENT_BINARY_DIR}/.obj/${ARCH}/")
set(LINKSCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/link.ld")
set(TARGETSPEC "${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/target.json")

set(LINKFLAGS "-T ${LINKSCRIPT}")
set(LINKFLAGS "${LINKFLAGS} -Map ${OBJDIR}map.txt")
set(LINKFLAGS "${LINKFLAGS} --gc-sections")
set(LINKFLAGS "${LINKFLAGS} -z max-page-size=0x1000")

set(RUSTFLAGS "--cfg arch__${ARCH} -C soft-float")
set(RUSTFLAGS "${RUSTFLAGS} -C panic=abort")

file(MAKE_DIRECTORY ${OBJDIR})

# Compile rust kernel object
add_custom_command(
    OUTPUT ${OBJDIR}kernel.a
    COMMAND ${CARGO} build -Z build-std=core --target=${TARGETSPEC} --release
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/target/target/release/libkernel.a
        ${OBJDIR}kernel.a
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${TARGETSPEC} Makefile
)

# Compile architecture's assembly stub
add_custom_command(
    OUTPUT ${OBJDIR}start.o
    COMMAND ${AS} ${ASFLAGS} -o ${OBJDIR}start.o
        ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/start.S
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/start.S Makefile
)

set(OBJS "${OBJDIR}start.o" "${OBJDIR}kernel.a")
set(BIN "${CMAKE_CURRENT_BINARY_DIR}/../kernel.${ARCH}.bin")

add_custom_command(
    OUTPUT ${BIN}
    COMMAND ${LD} -o ${BIN} ${LINKFLAGS} ${OBJS}
    COMMAND ${OBJDUMP} -S ${BIN} > ${BIN}.dsm
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${BIN}.elf64
        ${BIN}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${OBJS} ${LINKSCRIPT}
)


add_custom_target(kernel ALL DEPENDS ${BIN})

add_custom_target(clean_kernel
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${OBJDIR}
    COMMAND ${CMAKE_COMMAND} -E remove ${BIN} ${BIN}.dsm ${BIN}.elf64
)