; *******************************************************
; cpu.inc
; - Define variables
; 
%ifndef CPU_INC_
%define CPU_INC_
DetectCPU:
    ; Time to check whether or not your processor supports CPUID
    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21) in
    ; the FLAGS register. If we can flip it, CPUID is available.
    ; Copy FLAGS in to EAX via stack
    PUSHFD
    POP     eax
    ; Copy to ECX as well for comparing later on
    MOV     ecx, eax
    ; Flip the ID bit
    XOR     eax, ( 1 << 21 )
    ; Copy EAX to FLAGS via the stack
    PUSH    eax
    POPFD
    ; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    PUSHFD
    POP     eax
    ; Restore FLAGS from the old version stored in ECX (i.e. flipping the ID bit
    ; back if it was ever flipped).
    PUSH    ecx
    POPFD
    ; Compare EAX and ECX. If they are equal then that means the bit wasn't
    ; flipped, and CPUID isn't supported.
    XOR     eax, ecx
    JZ  DetectFailure.NoCPUID
    ; "cpuid" retrieve the information about your cpu
    ; eax=0x80000000: Get Highest Extended Function Implemented (only in long mode)
    ; 0x80000000 = 2^31
    MOV     eax,0x80000000
    CPUID
    ; Long mode can only be detected using the extended functions of CPUID (> 0x80000000)
    ; It is less, there is no long mode.
    CMP     eax,0x80000001
    ; if eax < 0x80000001 => CF=1 => jb
    ; jb, jump if below for unsigned number
    ; (jl, jump if less for signed number)
    JB      DetectFailure.NoLongMode
    ; eax=0x80000001: Extended Processor Info and Feature Bits
    MOV     eax,0x80000001
    CPUID

DetectFailure:
    MOV         si, ErrorMsg
    
    CALL        Puts16                          ; Error message
    
    MOV         ah, 0x00
    
    INT         0x16                            ; SMACK YOUR ASS ON THAT KEYBOARD AGAIN
    
    INT         0x19                            ; Reboot and try again, bios uses int 0x19 to find a bootable device

    .NoCPUID:
        MOV     si, ErrorMsgCPUID

        CALL    Puts16

        MOV         ah, 0x00
    
        INT         0x16                            ; SMACK YOUR ASS ON THAT KEYBOARD AGAIN
    
        INT         0x19                            ; Reboot and try again, bios uses int 0x19 to find a bootable device

    .NoLongMode:
        MOV     si, ErrorMsgLongMode

        CALL    Puts16
        
        MOV         ah, 0x00
    
        INT         0x16                            ; SMACK YOUR ASS ON THAT KEYBOARD AGAIN
    
        INT         0x19                            ; Reboot and try again, bios uses int 0x19 to find a bootable device
    
    .5_level_paging:
        MOV     si, ErrorMsgLevel5Paging

        CALL    Puts16
        
        MOV         ah, 0x00
    
        INT         0x16                            ; SMACK YOUR ASS ON THAT KEYBOARD AGAIN
    
        INT         0x19                            ; Reboot and try again, bios uses int 0x19 to find a bootable device
ErrorMsg:                   DB  "Hahahaha, Fuck you, but at Stage2"
ErrorMsgLongMode:           DB  "Long Mode Is Not Supported On This Machine"
ErrorMsgCPUID:              DB  "Processor does not support CPUID"
%endif