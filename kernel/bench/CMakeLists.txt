# Kernel benchmarks, only with -DINCRO_BENCH=ON (see bench.h)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bench.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND gcc -march=x86-64 -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -DINCRO_BENCH -c ${CMAKE_CURRENT_SOURCE_DIR}/bench.c -o ${CMAKE_BINARY_DIR}/bench.o
    COMMENT "Compiling kernel benchmarks"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bench.c ${CMAKE_CURRENT_SOURCE_DIR}/bench.h
)
add_custom_target(Bench ALL DEPENDS ${CMAKE_BINARY_DIR}/bench.o)
