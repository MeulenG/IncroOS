BUILD_DIR	=			../build/bootloader

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f

all			: 			$(BUILD_DIR)/Kernel_X86.bin

$(BUILD_DIR)/Kernel_X86.bin: Kernel_X86.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

clean:
	rm -rf $(BUILD_DIR)/Kernel_X86.bin