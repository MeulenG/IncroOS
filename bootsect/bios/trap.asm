
section .text
extern handler ;function isnt defined here
global vector0
global vector1
global vector2
global vector3
global vector4
global vector5
global vector6
global vector7
global vector8
global vector10
global vector11
global vector12
global vector13
global vector14
global vector16
global vector17
global vector18
global vector19
global vector32
global vector39
global eoi
global read_isr
global load_idt

Trap: 
    ;Save the state of the CPU
    push rax
    push rbx  
    push rcx
    push rdx  	  
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    inc byte[0xb8020]
    mov byte[0xb8021],0xe

    mov rdi,rsp
    call handler

TrapReturn:
    ;Restore the general purpose registers
    pop	r15
    pop	r14
    pop	r13
    pop	r12
    pop	r11
    pop	r10
    pop	r9
    pop	r8
    pop	rbp
    pop	rdi
    pop	rsi  
    pop	rdx
    pop	rcx
    pop	rbx
    pop	rax  

    add rsp,16 ;since we push 16 bytes its important to add it so that rsp holds its original value
    iretq



vector0:
    push 0 ;Error code
    push 0 ;Index Value/first exception
    jmp Trap

vector1:
    push 0
    push 1 ;Push index 1 on the stack for first exception
    jmp Trap

vector2:
    push 0
    push 2
    jmp Trap

vector3:
    push 0
    push 3	
    jmp Trap 

vector4:
    push 0
    push 4	
    jmp Trap   

vector5:
    push 0
    push 5
    jmp Trap    

vector6:
    push 0
    push 6	
    jmp Trap

vector7:
    push 0
    push 7	
    jmp Trap  

vector8:
    push 8 ;No need to include the error code, CPU pushes it on the stack when the error/interrupt happens
    jmp Trap  
;vector9 is reserved
vector10:
    push 10	
    jmp Trap 
                   
vector11:
    push 11	
    jmp Trap
    
vector12:
    push 12	
    jmp Trap          
          
vector13:
    push 13	
    jmp Trap
    
vector14:
    push 14	
    jmp Trap 
;15 is reserved
vector16:
    push 0
    push 16	
    jmp Trap          
          
vector17:
    push 17	
    jmp Trap                         
                                                          
vector18:
    push 0
    push 18	
    jmp Trap 
                   
vector19:
    push 0
    push 19	
    jmp Trap
;20-31 is reserved
vector32:
    push 0
    push 32
    jmp Trap

vector39: ;spurious interrupt
    push 0
    push 39
    jmp Trap

eoi:
    mov al,0x20
    out 0x20,al
    ret

read_isr:
    mov al,11
    out 0x20,al
    in al,0x20
    ret

load_idt:
    lidt [rdi] ;data is stored in rdi
    ret


