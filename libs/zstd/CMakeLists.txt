project(ZstdLib)

# zstddeclib.c is included by zstd_kernel.c, which routes its allocations to the kernel heap.
# SSE and MMX stay off because the bootloader never enables them (CR4.OSFXSR),
# and DYNAMIC_BMI2=0 keeps real hardware on the same code path as Bochs.
# -fno-reorder-functions stops -O2 from emitting .text.unlikely, which the linker would put
# before _start at 0x100000.
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/zstd_kernel.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -march=x86-64 -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -mno-mmx -mno-sse -O2 -fno-reorder-functions -DDYNAMIC_BMI2=0 -c ${CMAKE_CURRENT_SOURCE_DIR}/zstd_kernel.c -o ${CMAKE_BINARY_DIR}/zstd_kernel.o
    COMMENT "Compiling zstd decoder"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/zstd_kernel.c ${CMAKE_CURRENT_SOURCE_DIR}/zstddeclib.c ${CMAKE_CURRENT_SOURCE_DIR}/zstd_test_vector.h
)
add_custom_target(ZstdLib ALL DEPENDS ${CMAKE_BINARY_DIR}/zstd_kernel.o)
