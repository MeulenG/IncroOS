ORG     0xA000

ALIGN   32

BITS    32

main:       JMP Kernel_32
;*******************************************************
;	Defines
;*******************************************************
%define DATA_SEGMENT    0x10
;*******************************************************
;	Preprocessor directives 32-BIT MODE
;*******************************************************
%include "../SysBoot/Fat-Stage2/asmlib32.inc"
;******************************************************
;	ENTRY POINT For Kernel 32-Bit Mode
;******************************************************
Kernel_32:
    ;-------------------------------;
	;   Setup segments and stack	;
	;-------------------------------;
    MOV     ax,DATA_SEGMENT
    
    MOV     ds,ax
    
    MOV     es,ax
    
    MOV     ss,ax
    
    MOV     esp,0x7C00

	MOV		ebx, KRNLDR_HandOver

	CALL	Puts32

    JMP     Kernel_64_Main

Kernel_32_End:
    HLT
    
    JMP     Kernel_32_End

ALIGN   64

BITS    64
;*******************************************************
;	Preprocessor directives 64-BIT MODE
;*******************************************************
%include "../SysBoot/Fat-Stage2/asmlib64.inc"
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

    MOV     rsp,0x7C00


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