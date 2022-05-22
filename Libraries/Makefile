BUILD_DIR	=			../build

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


all			: 			$(BUILD_DIR)/libasm.o libasm.o $(BUILD_DIR)/print.o print.o $(BUILD_DIR)/memory.o memory.o $(BUILD_DIR)/syscall.o syscall.o $(BUILD_DIR)/lib.o lib.o Userprint.o

$(BUILD_DIR)/libasm.o: lib.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@


libasm.o: lib.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

$(BUILD_DIR)/print.o: printLib.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c printLib.c


print.o: printLib.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c printLib.c

memory.o: memLib.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c memLib.c


$(BUILD_DIR)/memory.o: memLib.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c memLib.c

lib.o: lib.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c lib.c

Userprint.o: Userprint.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c Userprint.c

$(BUILD_DIR)/lib.o: lib.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c lib.c

$(BUILD_DIR)/syscall.o: Syscall.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

syscall.o: Syscall.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@


clean:
	rm -rf $(BUILD_DIR)/libasm.o
	rm -rf libasm.o
	rm -rf $(BUILD_DIR)/printLib.o
	rm -rf printLib.o
	rm -rf $(BUILD_DIR)/memLib.o
	rm -rf memLib.o
	rm -rf $(BUILD_DIR)/syscall.o
	rm -rf syscall.o
	rm -rf $(BUILD_DIR)/Userprint.o
	rm -rf Userprint.o
	rm -rf lib.o