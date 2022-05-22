BUILD_DIR	=			../build

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


all			: 			$(BUILD_DIR)/cputrapasm.o cputrapasm.o $(BUILD_DIR)/cputrap.o cputrap.o

$(BUILD_DIR)/cputrapasm.o: cputrap.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

cputrapasm.o: cputrap.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

$(BUILD_DIR)/cputrap.o: cputrap.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c cputrap.c

cputrap.o: cputrap.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c cputrap.c


clean:
	rm -rf cputrapasm.o
	rm -rf cputrap.o
	rm -rf $(BUILD_DIR)/cputrap.o
	rm -rf $(BUILD_DIR)/cputrapasm.o