BUILD_DIR	=			../build

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f



all: $(BUILD_DIR)/Stage1.bin

$(BUILD_DIR)/Stage1.bin: Stage1.asm
	nasm Stage1.asm -f bin -o Stage1.bin


clean:
	rm $(BUILD_DIR)/*