BUILD_DIR	=			../build/bootloader

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f

all			: 			$(BUILD_DIR)/FAT-STAGE1.bin FAT-STAGE1.bin

$(BUILD_DIR)/FAT-STAGE1.bin: FAT-STAGE1.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

FAT-STAGE1.bin: FAT-STAGE1.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

clean:
	rm -rf $(BUILD_DIR)/FAT-STAGE1.bin
	rm -rf FAT-STAGE1.bin