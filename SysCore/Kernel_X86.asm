ALIGN   64
BITS    64
ORG     0x200000

Kernel_Stage:
    MOV     byte[0xb8000],'K'
    
    MOV     byte[0xb8001],0xA

Kernel_End:
    HLT
    JMP Kernel_End