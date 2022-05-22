
section .text
extern isr_handler
global INTERRUPT0
global INTERRUPT1
global INTERRUPT2
global INTERRUPT3
global INTERRUPT4
global INTERRUPT5
global INTERRUPT6
global INTERRUPT7
global INTERRUPT8
global INTERRUPT10
global INTERRUPT11
global INTERRUPT12
global INTERRUPT13
global INTERRUPT14
global INTERRUPT16
global INTERRUPT17
global INTERRUPT18
global INTERRUPT19
global INTERRUPT32
global INTERRUPT33
global INTERRUPT39
global sysint
global ENDOFINT
global READ_ISR
global idt_Install
global load_cr3
global pstart
global read_cr2
global swap
global TrapReturn
global in_byte

Trap:
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

    mov rdi,rsp
    call isr_handler

TrapReturn:
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

    add rsp,16
    iretq



INTERRUPT0:
    push 0
    push 0
    jmp Trap

INTERRUPT1:
    push 0
    push 1
    jmp Trap

INTERRUPT2:
    push 0
    push 2
    jmp Trap

INTERRUPT3:
    push 0
    push 3	
    jmp Trap 

INTERRUPT4:
    push 0
    push 4	
    jmp Trap   

INTERRUPT5:
    push 0
    push 5
    jmp Trap    

INTERRUPT6:
    push 0
    push 6	
    jmp Trap      

INTERRUPT7:
    push 0
    push 7	
    jmp Trap  

INTERRUPT8:
    push 8
    jmp Trap  

INTERRUPT10:
    push 10	
    jmp Trap 
                   
INTERRUPT11:
    push 11	
    jmp Trap
    
INTERRUPT12:
    push 12	
    jmp Trap          
          
INTERRUPT13:
    push 13	
    jmp Trap
    
INTERRUPT14:
    push 14	
    jmp Trap 

INTERRUPT16:
    push 0
    push 16	
    jmp Trap          
          
INTERRUPT17:
    push 17	
    jmp Trap                         
                                                          
INTERRUPT18:
    push 0
    push 18	
    jmp Trap 
                   
INTERRUPT19:
    push 0
    push 19	
    jmp Trap

INTERRUPT32:
    push 0
    push 32
    jmp Trap

INTERRUPT33:
    push 0
    push 33
    jmp Trap

INTERRUPT39:
    push 0
    push 39
    jmp Trap

sysint:
    push 0
    push 0x80
    jmp Trap

ENDOFINT:
    mov al,0x20
    out 0x20,al
    ret

READ_ISR:
    mov al,11
    out 0x20,al
    in al,0x20
    ret

idt_Install:
    lidt [rdi]
    ret

load_cr3:
    mov rax,rdi
    mov cr3,rax
    ret

read_cr2:
    mov rax,cr2
    ret

pstart:
    mov rsp,rdi
    jmp TrapReturn

swap:
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    
    mov [rdi],rsp
    mov rsp,rsi
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    
    ret 

in_byte:
    mov rdx,rdi
    in al,dx
    ret   