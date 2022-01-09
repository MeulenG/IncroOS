include 				/home/puhaa/Desktop/PuhaaOS/buildpath.txt

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


all: 					$(BUILD_DIR)/Stage1.bin

$(BUILD_DIR)/Stage2.bin: Stage2.asm
	${ASMC} $< ${ASMC_ARGS} bin -o $@


clean:
	rm $(BUILD_DIR)/*