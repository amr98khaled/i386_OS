                                                                        # set symbol,expression-->Set the value of symbol to expression.You may set a symbol many times in the same assembly provided that the values given to the symbol are constants.
                                                                        #The symbols defined in set section below won't be put in the loader.o file 

.set MAGIC, 0x1badb002              
                                                                        #Magic number so that the bootloader recognizes our file as the kernel so to look at loader.s file to know that we want it to be used in boot. 
                                                                        #_If bit 0 in the ‘flags’ word is set,then all boot modules loaded along with the operating system must be aligned on page (4KB) boundaries. Some operating systems
.set FLAGS, (1<<0 | 1<<1)          
                                   
                                                                        #expect to be able to map the pages containing boot modules directly into a paged address space during startup, and thus need the boot modules to be page-aligned.
                                                                        #_If bit 1 in the ‘flags’ word is set, then information on available memory via at least the ‘mem_*’ fields of the Multiboot information structure must be included.  ;If the boot loader is capable of #passing a memory map (the ‘mmap_*’ fields) and one exists, then it may be included as well.

.set CHECKSUM, -(MAGIC+FLAGS)       
                                                                        #The field ‘checksum’ is a 32-bit unsigned value which, when added to the other magic fields (i.e. ‘magic’ and ‘flags’), must have a 32-bit unsigned sum of zero.

                                                                        #We have to put the above symbols in a loader.o file 
                                    
.section .multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM
                                    
.section .text
.extern kernelMain
.extern callConstructors
.global loader
                                    
                                                                        #When the bootloader has decided that this file is the kernel it stores some information[size of ram,...etc]  on the ram in a so called multiboot structure which is then pointed to by the ax register , ax --> multiboot struct
                                                                        #The magic number is copied to the bx register
loader:
    mov $kernel_stack, %esp
    call callConstructors                                               #initialize global constructors after initializing the stack and before calling kernelMain 
    push %eax
    push %ebx
    call kernelMain 
                                                                        # the code is not supposed to get out of kernelMain because of while(1) but to make sure we will write the _stop procedure below
    
_stop:
    cli 
                                                                        #Clears the interrupt flag if the current privilege level is at least as privileged as IOPL; affects no other flags. External interrupts disabled at the end of the cli instruction or from that point on until the interrupt flag is set.
    hlt 
                                                                        # halts the cpu until the next external interrupt is fired 
    jmp _stop
    
.section .bss
.space 2*1024*1024 
                                                                        #2Megabytes of memory to be left free between the current written memory and the location where the stack pointer is at so when we push things to the stack we don't overwrite things in memoery 
kernel_stack:
