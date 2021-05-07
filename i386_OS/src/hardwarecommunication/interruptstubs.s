.set IRQ_BASE, 0x20

.section .text

.extern _ZN4myos21hardwarecommunication16InterruptManager15HandleInterruptEhj                  #name of InterruptManager::HandleInterrupt() function located in interrupts.cpp after compiling and mangling 

.macro HandleException num                                         #num is parameter for the macro
.global _ZN4myos21hardwarecommunication16InterruptManager19HandleException\num\()Ev         
#name of InterruptManager::HandleException0x..() function located in interrupts.cpp after compiling and mangling and adding num parameter to the function name
_ZN4myos21hardwarecommunication16InterruptManager19HandleException\num\()Ev:
    movb $\num, (interruptnumber)
    jmp int_bottom
.endm


.macro HandleInterruptRequest num
.global _ZN4myos21hardwarecommunication16InterruptManager26HandleInterruptRequest\num\()Ev #name of InterruptManager::HandleInterruptRequest0x..() function located in interrupts.cpp after compiling and mangling and adding num parameter to the function name
_ZN4myos21hardwarecommunication16InterruptManager26HandleInterruptRequest\num\()Ev:
    movb $\num + IRQ_BASE, (interruptnumber)
    pushl $0                                                                #push 0 for the error variable in CPUstate struct , 0 is filler for the error code. For an exception, the processor pushes an error value automatically but for an interrupt we have to push a value ourselves.
    jmp  int_bottom
.endm




#We call the following macros with their parameters which is the interrupt number . Each line of the following macros gets replaced by the above code so macros are used to decrease duplication of code .

#To have a better understanding :
#_if you notice in the makefile interruptstubs.o is linked in the objects variable before interrupts.o , so in the final binary file the definitions of the handlers are above their usage in the SetInterruptDescriptorTableEntry() .

#so for example : we have this macro HandleInterruptRequest 0x00 in interruptstubs.s so it is converted to a definition in assembler to this :
#   .global _ZN4myos21hardwarecommunication16InterruptManager26HandleInterruptRequest\0x00\()Ev
#  _ZN4myos21hardwarecommunication16InterruptManager26HandleInterruptRequest\0x00\()Ev: 
#     movb $0x00 + IRQ_BASE, (interruptnumber) 
#    jmp int_bottom
        
#So when we are in interrupts.cpp and use SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x00, CodeSegment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE); , the address of the function HandleInterruptRequest0x00 is put in IDT . so when an IRQ0 occurs the CPU calls this function which when compiled is mangled to this name _ZN4myos21hardwarecommunication16InterruptManager26HandleInterruptRequest\0x00\()Ev because we are not using the extern keyword .



HandleException 0x00
HandleException 0x01
HandleException 0x02
HandleException 0x03
HandleException 0x04
HandleException 0x05
HandleException 0x06
HandleException 0x07
HandleException 0x08
HandleException 0x09
HandleException 0x0A
HandleException 0x0B
HandleException 0x0C
HandleException 0x0D
HandleException 0x0E
HandleException 0x0F
HandleException 0x10
HandleException 0x11
HandleException 0x12
HandleException 0x13

HandleInterruptRequest 0x00
HandleInterruptRequest 0x01
HandleInterruptRequest 0x02
HandleInterruptRequest 0x03
HandleInterruptRequest 0x04
HandleInterruptRequest 0x05
HandleInterruptRequest 0x06
HandleInterruptRequest 0x07
HandleInterruptRequest 0x08
HandleInterruptRequest 0x09
HandleInterruptRequest 0x0A
HandleInterruptRequest 0x0B
HandleInterruptRequest 0x0C
HandleInterruptRequest 0x0D
HandleInterruptRequest 0x0E
HandleInterruptRequest 0x0F
HandleInterruptRequest 0x31
HandleInterruptRequest 0x80

int_bottom:

  #  pusha
  #  pushl %ds
  #  pushl %es
  #  pushl %fs
  #  pushl %gs
  
    pushl %ebp
    pushl %edi
    pushl %esi

    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax


  
    #C++ function call
    #push parameters of InterruptManager::HandleInterrupt(interruptnumber,esp)
    pushl %esp
    push (interruptnumber)
    call _ZN4myos21hardwarecommunication16InterruptManager15HandleInterruptEhj
    
    #switch the stack of the new task by loading a new value to esp and not by popping the previous value of esp that was for the stack of the previous task
    # add   %esp, 6
    # mov   %eax, %esp
    
   # pop %gs
   # pop %fs
   # pop %es
   # pop %ds
   # popa
   
    #The value returned from the call which is esp is returned in %eax so we move this value in %esp
    mov %eax, %esp # switch the stack
    
    # restore registers
    popl %eax
    popl %ebx
    popl %ecx
    popl %edx

    popl %esi
    popl %edi
    popl %ebp
    
    add $4, %esp
    #_if it is "add $8, %esp " -> drop err and irq num but i don't know why we add 4 to esp
    
.global _ZN4myos21hardwarecommunication16InterruptManager15InterruptIgnoreEv    #name of InterruptManager::InterruptIgnore() function located in interrupts.cpp after compiling and mangling 
_ZN4myos21hardwarecommunication16InterruptManager15InterruptIgnoreEv:
        iret                                        #handler coming from int_bottom or InterruptIgnore will return from here
        
        
.data  
    interruptnumber: .byte 0
    
    
    
    
