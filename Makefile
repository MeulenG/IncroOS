BUILD_DIR=build
BOOTLOADER=$(BUILD_DIR)/boot/bootsect.o
OS=$(BUILD_DIR)/os/kernel_entry.o

C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c libc/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h libc/*.h)

OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o} 

# Environmental Variables
CC = /home/puhaa/opt/cross/bin/i686-elf-gcc
GDB = /home/puhaa/opt/cross/bin/i686-elf-gdb


all: os-image.bin

.PHONY: os-image.bin bootloader os


CFLAGS = -g -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs \
		 -Wall -Wextra -Werror


bootloader:
	make -C bootloader

os-image.bin: boot/bootsect.bin kernel.bin
	cat $^ > os-image.bin

# Only relevant for debugging
kernel.elf: boot/kernel_entry.o ${OBJ}
	i686-elf-ld -o $@ -Ttext 0x1000 $^ 


kernel.bin: boot/kernel_entry.o ${OBJ}
	i686-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

os:
	make -C os

run: os-image.bin
	qemu-system-i386 -fda os-image.bin

debug: os-image.bin kernel.elf
	qemu-system-i686 -s -fda os-image.bin -d guest_errors,int &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"


%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -ffreestanding -c $< -o $@

clean:
	rm -rf *.bin *.dis *.o os-image.bin *.elf
	rm -rf kernel/*.o boot/*.bin drivers/*.o boot/*.o cpu/*.o libc/*.o


clean:
	make -C bootloader clean
	make -C os clean
