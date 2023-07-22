%ifndef __A20_INC
%define __A20_INC

bits	16
;--------------------------------------
; Enables a20 line through bios
;--------------------------------------

EnableA20_Bios:
	PUSHA

	MOV	ax, 0x2401
	
    INT	0x15
	
    POPA
	
    RET

%endif