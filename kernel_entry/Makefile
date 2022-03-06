# Environmental Variables
CC  		= 			/home/puhaa/opt/cross/bin/i686-elf-gcc
GDB 		= 			/home/puhaa/opt/cross/bin/i686-elf-gdb
BUILD_DIR	=			../build


#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f

all				: 		$(BUILD_DIR)/kernel_entry.o


$(BUILD_DIR)/kernel_entry.o: kernel_entry.asm
	${ASMC} kernel_entry.asm ${ASMC_ARGS} elf64 -o $@


clean:
	rm $(BUILD_DIR)/*