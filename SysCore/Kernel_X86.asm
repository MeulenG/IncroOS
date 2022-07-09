ORG     0xA000
ALIGN   64
BITS    64


Kernel_Stage:
    MOV     byte[0xb8000],'K'
    
    MOV     byte[0xb8001],0xB

Kernel_End:
    HLT
    JMP Kernel_End