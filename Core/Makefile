BUILD_DIR_KRNLDR	=			../build/OS
BUILD_DIR			=			../build

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


all			: 			$(BUILD_DIR_KRNLDR)/kernel.o kernel.o $(BUILD_DIR)/main.o main.o $(BUILD_DIR)/kerneldebugger.o kerneldebugger.o $(BUILD_DIR)/process.o process.o $(BUILD_DIR)/syscall.o syscall.o

$(BUILD_DIR_KRNLDR)/kernel.o: kernel.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@


kernel.o: kernel.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

$(BUILD_DIR)/main.o: main.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c main.c

main.o: main.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c main.c

kerneldebugger.o: kerneldebugger.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c kerneldebugger.c


$(BUILD_DIR)/kerneldebugger.o: kerneldebugger.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c kerneldebugger.c

$(BUILD_DIR)/process.o: process.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c process.c

process.o: process.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c process.c

$(BUILD_DIR)/syscall.o: syscall.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c syscall.c

syscall.o: syscall.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c syscall.c

clean:
	rm -rf kernel.bin
	rm -rf $(BUILD_DIR_KRNLDR)/kernel.o
	rm -rf kernel.o
	rm -rf $(BUILD_DIR)/main.o
	rm -rf $(BUILD_DIR)/debug.o
	rm -rf main.o
	rm -rf kerneldebugger.o
	rm -rf process.o
	rm -rf syscall.o
