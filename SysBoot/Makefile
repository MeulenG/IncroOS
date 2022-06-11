BUILD_DIR	=			../build/bootloader

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f

all			: 			$(BUILD_DIR)/FAT-STAGE1.bin $(BUILD_DIR)/FAT-STAGE2.bin

$(BUILD_DIR)/FAT-STAGE1.bin: Fat-Stage1/FAT-STAGE1.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

$(BUILD_DIR)/FAT-STAGE2.bin: Fat-Stage2/FAT-STAGE2.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

clean:
	rm -rf $(BUILD_DIR)/FAT-STAGE1.bin
	rm -rf $(BUILD_DIR)/FAT-STAGE2.bin