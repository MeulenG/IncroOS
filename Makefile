C_SOURCES 	= 			$(wildcard kernel/*.c drivers/*.c cpu/*.c libc/*.c)
HEADERS 	= 			$(wildcard kernel/*.h drivers/*.h cpu/*.h libc/*.h)
# Nice syntax for file extension replacement
OBJ 		= 			${C_SOURCES:.c=.o cpu/interrupt.o} 



# Environmental Variables
CC  		= 			/home/puhaa/opt/cross/bin/i686-elf-gcc
GDB 		= 			/home/puhaa/opt/cross/bin/i686-elf-gdb


# Emulator
EMU 	  	= 			qemu-system-i386
EMU_ARGS  	= 			-machine q35
EMU_ARGS  	+= 			-fda


# -g: Use debugging symbols in gcc
CFLAGS 	 	= 			-g
GCC_ARGS 	= 			-ffreestanding
GCC_ARGS 	+= 			-Wall
GCC_ARGS 	+= 			-Wextra
GCC_ARGS 	+= 			-Werror
GCC_ARGS 	+= 			-fno-exceptions
GCC_ARGS 	+= 			-m32
GCC_ARGS 	+= 			-fno-builtin
GCC_ARGS 	+= 			-nostdlib
GCC_ARGS 	+= 			-nostdinc
GCC_ARGS 	+= 			-fno-stack-protector
GCC_ARGS 	+= 			-nostartfiles
GCC_ARGS 	+= 			-nodefaultlibs

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


RMVE 		= 			rm -rf

# First rule is run by default
PuhaaOS-image.bin: boot/Stage1.bin kernel.bin
	cat $^ > PuhaaOS-image.bin

# '--oformat binary' deletes all symbols as a collateral, so we don't need
# to 'strip' them manually on this case
kernel.bin: boot/kernel_entry.o ${OBJ}
	i686-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

# Used for debugging purposes
kernel.elf: boot/kernel_entry.o ${OBJ}
	i686-elf-ld -o $@ -Ttext 0x1000 $^ 

run: PuhaaOS-image.bin
	${EMU} ${EMU_ARGS} PuhaaOS-image.bin

# Open the connection to qemu and load our kernel-object file with symbols
debug: PuhaaOS-image.bin kernel.elf
	qemu-system-i386 -s -fda PuhaaOS-image.bin -d guest_errors,int &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

# Generic rules for wildcards
# To make an object, always compile from its .c
%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} ${GCC_ARGS} -c $< -o $@

%.o: %.asm
	${ASMC} $< ${ASMC_ARGS} elf -o $@

%.bin: %.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

clean:
	${RMVE} *.bin *.dis *.o os-image.bin *.elf
	${RMVE} kernel/*.o boot/*.bin drivers/*.o boot/*.o cpu/*.o libc/*.o