ALIGN   64

BITS    64
;*******************************************************
;	Preprocessor directives 64-BIT MODE
;*******************************************************
%include "asmlib64.inc"
%include "Gdt.inc"
;******************************************************
;	ENTRY POINT For Kernel 64-Bit Mode
;******************************************************
Kernel_64_Main:
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    MOV     ax, DATA_SEGMENT
    
    MOV     ds, ax
    
    MOV     es, ax
    
    MOV     fs, ax
    
    MOV     gs, ax
    
    MOV     ss, ax

    MOV     rsp,0xA000
    
Kernel_64_End:
    HLT
    
    JMP     Kernel_64_End

;*******************************************************
;	Data Section
;*******************************************************
KRNLDR_HandOver				DB 	0x0A, 0x0D "Control Handed To Kernel, Sucessfully", 0x00

;*******************************************************
;	Data Error Section
;*******************************************************
KRNLDR_Error                DB  0x0A, 0x0D "Control Handed To Kernel, Unsucessfully", 0x00