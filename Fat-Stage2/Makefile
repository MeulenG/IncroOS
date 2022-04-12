BUILD_DIR	=			../build/bootloader

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


all			: 			$(BUILD_DIR)/fat-stage2.bin	KRNLDR.SYS

$(BUILD_DIR)/fat-stage2.bin: fat-stage2.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

KRNLDR.SYS: fat-stage2.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@

clean:
	rm -rf LOADER.SYS