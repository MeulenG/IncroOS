BUILD_DIR	=			../build/bootloader

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f

all			: 			$(BUILD_DIR)/Stage1_X86.bin $(BUILD_DIR)/Stage2_X86.bin

$(BUILD_DIR)/Stage1_X86.bin: Fat-Stage1/Stage1_X86.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

$(BUILD_DIR)/Stage2_X86.bin: Fat-Stage2/Stage2_X86.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

clean:
	rm -rf $(BUILD_DIR)/Stage1_X86.bin
	rm -rf $(BUILD_DIR)/Stage2_X86.bin