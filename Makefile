GFLAGS=-m32
CCFLAGS =-std=c11 -02 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing
CCFLAGS+=-Wno-pointer-arith -Wno-newline-eof -Wno-unused-parameter
ASFLAGS=

LDFLAGS=-nostdlib -ffreestanding

