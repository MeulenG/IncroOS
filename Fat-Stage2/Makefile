BUILD_DIR	=			../build/bootloader

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


all			: 			$(BUILD_DIR)/FAT-STAGE2.bin FAT-STAGE2.bin

$(BUILD_DIR)/FAT-STAGE2.bin: FAT-STAGE2.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

FAT-STAGE2.bin: FAT-STAGE2.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

clean:
	rm -rf $(BUILD_DIR)/FAT-STAGE2.bin
	rm -rf FAT-STAGE2.bin