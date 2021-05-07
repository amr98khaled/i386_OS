/*
 Steps :
1- We set the Interrupt handlers.
2- Set IDT
3- Send commands to the PICs through ICW to tell them to send interrupts to the CPU cause everything is set.
4- Tell the CPU to use the IDT : asm volatile("lidt %0" : : "m" (idt_pointer));
 */

#include <hardwarecommunication/interrupts.h>
using namespace myos;
using namespace myos::common;
using namespace myos::hardwarecommunication;

void printf(char* str);
void printfHex(uint8_t);

InterruptHandler::InterruptHandler(InterruptManager* interruptManager, uint8_t InterruptNumber)
{
    this->InterruptNumber = InterruptNumber;
    this->interruptManager = interruptManager;
    interruptManager->handlers[InterruptNumber] = this;
    //this holds the base address of objects defined in kernel.cpp like keyboard and mouse objects which are both derived from the InterruptHandler class . 
}

InterruptHandler::~InterruptHandler()
{
    if(interruptManager->handlers[InterruptNumber]==this)
        interruptManager->handlers[InterruptNumber]=0;
}

uint32_t InterruptHandler::HandleInterrupt(uint32_t esp)
{
    return esp;
}

InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];
InterruptManager* InterruptManager::ActiveInterruptManager=0;

void InterruptManager::SetInterruptDescriptorTableEntry(uint8_t interrupt, uint16_t CodeSegment, void (*handler)(), uint8_t DescriptorPrivilegeLevel, uint8_t DescriptorType)
{
    // address of pointer to code segment (relative to global descriptor table)
    // and address of the handler (relative to segment)
    interruptDescriptorTable[interrupt].handlerAddressLowBits = ((uint32_t)handler) & 0xFFFF;
    interruptDescriptorTable[interrupt].handlerAddressHighBits = (((uint32_t)handler) >> 16) & 0xFFFF;
    interruptDescriptorTable[interrupt].gdt_codeSegmentSelector = CodeSegment;
    
    const uint8_t IDT_DESC_PRESENT = 0x80;
    interruptDescriptorTable[interrupt].access = IDT_DESC_PRESENT | ((DescriptorPrivilegeLevel & 3)<<5) | DescriptorType;
    interruptDescriptorTable[interrupt].reserved = 0;
}


InterruptManager::InterruptManager(uint16_t hardwareInterruptOffset, GlobalDescriptorTable* globalDescriptorTable, TaskManager* taskManager)
:programmableInterruptControllerMasterCommandPort(0x20),
programmableInterruptControllerMasterDataPort(0x21),
programmableInterruptControllerSlaveCommandPort(0xA0),
programmableInterruptControllerSlaveDataPort(0xA1)
{
    this->taskManager = taskManager;
    this->hardwareInterruptOffset = hardwareInterruptOffset;
    uint32_t CodeSegment = globalDescriptorTable-> CodeSegmentSelector();
    
    const uint8_t IDT_INTERRUPT_GATE = 0xE;                         //type of entry [0xE] : 80386 32-bit interrupt gate
    
    for(uint8_t i=255; i>0;--i)
    {
        SetInterruptDescriptorTableEntry(i,CodeSegment,&InterruptIgnore,0,IDT_INTERRUPT_GATE); //set handlers entries at first to InterruptIgnore
        handlers[i]=0; //set handlers to null
    }
    
    SetInterruptDescriptorTableEntry(0,CodeSegment,&InterruptIgnore,0,IDT_INTERRUPT_GATE); //set handler entry 0 at first to InterruptIgnore
    handlers[0]=0;
    
    
   SetInterruptDescriptorTableEntry(0x00, CodeSegment, &HandleException0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x01, CodeSegment, &HandleException0x01, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x02, CodeSegment, &HandleException0x02, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x03, CodeSegment, &HandleException0x03, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x04, CodeSegment, &HandleException0x04, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x05, CodeSegment, &HandleException0x05, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x06, CodeSegment, &HandleException0x06, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x07, CodeSegment, &HandleException0x07, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x08, CodeSegment, &HandleException0x08, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x09, CodeSegment, &HandleException0x09, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0A, CodeSegment, &HandleException0x0A, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0B, CodeSegment, &HandleException0x0B, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0C, CodeSegment, &HandleException0x0C, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0D, CodeSegment, &HandleException0x0D, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0E, CodeSegment, &HandleException0x0E, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x0F, CodeSegment, &HandleException0x0F, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x10, CodeSegment, &HandleException0x10, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x11, CodeSegment, &HandleException0x11, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x12, CodeSegment, &HandleException0x12, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x13, CodeSegment, &HandleException0x13, 0, IDT_INTERRUPT_GATE);


    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x00, CodeSegment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x01, CodeSegment, &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x02, CodeSegment, &HandleInterruptRequest0x02, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x03, CodeSegment, &HandleInterruptRequest0x03, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x04, CodeSegment, &HandleInterruptRequest0x04, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x05, CodeSegment, &HandleInterruptRequest0x05, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x06, CodeSegment, &HandleInterruptRequest0x06, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x07, CodeSegment, &HandleInterruptRequest0x07, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x08, CodeSegment, &HandleInterruptRequest0x08, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x09, CodeSegment, &HandleInterruptRequest0x09, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0A, CodeSegment, &HandleInterruptRequest0x0A, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0B, CodeSegment, &HandleInterruptRequest0x0B, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0C, CodeSegment, &HandleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0D, CodeSegment, &HandleInterruptRequest0x0D, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0E, CodeSegment, &HandleInterruptRequest0x0E, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(hardwareInterruptOffset + 0x0F, CodeSegment, &HandleInterruptRequest0x0F, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(                          0x80, CodeSegment, &HandleInterruptRequest0x80, 0, IDT_INTERRUPT_GATE);


    
    //ICW1 :  is used to tell the PICs if a ICW4 is following and if the PIC is working in a cascaded PIC environment (that means, if they communicate with each other).
    //bit 0 : if set, the PIC expects to recieve ICW4 during initialization.
    //bit 1 : If set, only one PIC in system. If cleared, PIC is cascaded with slave PICs, and ICW3 must be sent to controller.
    //bit 4 : Initialization bit. Set 1 if PIC is to be initialized
    programmableInterruptControllerMasterCommandPort.Write(0x11);
    programmableInterruptControllerSlaveCommandPort.Write(0x11);
    
    

    // ICW2 : This control word is used to map the base address of the IDT of which the PICs are to use.
    programmableInterruptControllerMasterDataPort.Write(hardwareInterruptOffset);
    programmableInterruptControllerSlaveDataPort.Write(hardwareInterruptOffset+8);
    
    
    // ICW3 : This command word is used to let the PIC controllers know how they are cascaded.
    // Notice that IRQ 2 is Bit 2 within ICW 3. So, in order to set IRQ 2, we need to set bit 2 (Which is at 0100 binary, or 0x4).
    //To send this to the secondary PIC, we must remember that we must send this in binary notation.Only Bits 0-2 are used to represent the IRQ line. Send (0x02)  
    programmableInterruptControllerMasterDataPort.Write(0x04);
    programmableInterruptControllerSlaveDataPort.Write(0x02);

    
    //ICW4 is used for telling that we are working in a 80x86 architecture and if the interruption is handled automatically or if it needs help from software.
    //bit 0 : if set, it is in 80x86 mode.
    programmableInterruptControllerMasterDataPort.Write(0x01);
    programmableInterruptControllerSlaveDataPort.Write(0x01);

    //OCW1 : This represents the value in the Interrupt Mask Register (IMR). It does not have a special format, so it is handled directly in the implementation file to enable and disable hardware interrupts. It is a single byte in size. We enable and disable ("mask and unmask") an interrupt request line by setting the correct bit.
    //Activate (unmask) all IRQS
    programmableInterruptControllerMasterDataPort.Write(0x00);
    programmableInterruptControllerSlaveDataPort.Write(0x00);

    
    InterruptDescriptorTablePointer idt_pointer;
    idt_pointer.size = 256*sizeof(GateDescriptor) - 1;
    idt_pointer.base = (uint32_t)interruptDescriptorTable;
    
    asm volatile("lidt %0" : : "m" (idt_pointer));
    
}



InterruptManager::~InterruptManager()
{
    Deactivate();
}

uint16_t InterruptManager::HardwareInterruptOffset()
{
    return hardwareInterruptOffset;
}

void InterruptManager::Activate()
{
    /* In theory there could be multiple InterruptManagers but this doesn't make sense because the processor has only one IDT . But we write the below code just in case for multiple InterruptManagers :
        if active interrupt manager has already been set we will deactivate the old one and set the new one */
    if(ActiveInterruptManager!=0)
    {
        ActiveInterruptManager->Deactivate();
    }
    ActiveInterruptManager=this;
        asm("sti");
}

void InterruptManager::Deactivate()
{
    if(ActiveInterruptManager==this)
    {
        ActiveInterruptManager=0;
        asm("cli");
    }
}

/* When an exception or interrupt occur , This HandleInterrupt() function is the one called from interruptstubs.s
HandleInterrupt() is defined as a static function in the InterruptManager class , and it calls DoHandleInterrupt() function which accesses members of the InterruptManager class such as the command and data ports . So for a static function like HandleInterrupt() it has to acess a static object so we define a pointer to a static object of the InterruptManager class which is ActiveInterruptManager. */
uint32_t InterruptManager::HandleInterrupt(uint8_t interrupt, uint32_t esp)
{
    if(ActiveInterruptManager!=0)
    {
        return ActiveInterruptManager->DoHandleInterrupt(interrupt,esp);
    }
//if no task switching -> it returns the current esp
    return esp;
}

uint32_t InterruptManager::DoHandleInterrupt(uint8_t interrupt, uint32_t esp)
{
    if(handlers[interrupt]!=0)
    {
        esp = handlers[interrupt]->HandleInterrupt(esp); 
        /*
         If handlers[interrupt] contains for example the address of a keyboard object then when it calls HandleInterrupt(esp) , the HandleInterrupt(esp) will resolve to the most derived class because it is a virtual function and because keyboard is derived from InterruptHandler class . So in this case HandleInterrupt(esp) which is found in keyboard class is called. But if handlers[interrupt] doesn't contain the address of a keyboard object then it calls the HandleInterrupt(esp) found in InterruptHandler class which just returns esp and it has no relation to the HandleInterrupt function that is called from interruptstubs.s
         */
    }
    else if (interrupt != hardwareInterruptOffset)
    {
        //This output should be printed if the interrupt is not the timer interrupt nor any other defined interrupt
        printf("UNHANDLED INTERRUPT 0x");
        printfHex(interrupt);
    }
    
    // interrupt is timer interrupt then let task manager call the scheduler to switch between tasks.
    if(interrupt == hardwareInterruptOffset)
    {
        esp = (uint32_t)taskManager->Schedule((CPUState*)esp);      //schedule will take the esp of current task , save state of current task , switch tasks , and return esp of the new task.
    }
    
    //hardware interrupts must be acknowledged
    if(hardwareInterruptOffset <= interrupt && interrupt < hardwareInterruptOffset+16)
    {
        programmableInterruptControllerMasterCommandPort.Write(0x20);
        if(hardwareInterruptOffset +8 <= interrupt)
        {
            programmableInterruptControllerSlaveCommandPort.Write(0x20);
        }
    }
    /* Acknowledgments must be sent to the Master PIC if the IRQ is triggered from the Master PIC and must be sent to both the Master and Slave PICs if the IRQ is triggered from the Slave PIC. After interrupts have been acknowledged , the PICs keep sending interrupts to the CPU */
    return esp;
    
}
