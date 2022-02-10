# Environmental Variables
include 				/home/puhaa/Desktop/OMOS/buildbootpath

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


all			: 			$(BUILD_DIR_BOOT)/fat-stage2.bin

$(BUILD_DIR_BOOT)/fat-stage2.bin: fat-stage2.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@
