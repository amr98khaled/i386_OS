 
                                                                                        /*multiboot_structure & multiboot_magic are passed to the kernelMain by the bootloader
                                                                                         * 
                                                                                        We are outside an operating system like windows or linux because we are building our own one , so the regular printf() is inside a dynamic library called glibc so when an operating system like linux or windows sees that our program calls printf() it dynamically links it to the glibc library .
                                                                                        Here in our operating system we are on our own meaning that we don't have glibc library and we don't have anything to connect the program we write to any library. We don't have dynamic linking nor libraries nor dynamic memory management so we can't use malloc() nor free(). We don't have any function to write something on the screen .Everything we want to do we have to write it ourselves. We have to write our own printf().
                                                                                        *
                                                                                        There is a location in Ram starting at 0xB8000[memory mapped I/O to the VGA controller] that whatever you put there will be written on the screen by the graphics card. 
                                                                                        You put a char at one byte and the consecutive byte contains a color information which is divided as follows : 4 bits are for the background color of the char displayed on the screen and the other 4 bits are for the foreground color of the same char being displayed . Then you put in the next byte the next char and then a byte for its color info and so on .
                                                                                        Initially the bytes specified for the color info have all the same value for displaying chars with white text on a black background . If we want the color of chars like this we just have to write the chars in their corresponding bytes in Ram and make sure that we don't overwrite the color info bytes. 
                                                                                        
                                                                                        The text screen video memory for colour monitors resides at 0xB8000, and for monochrome monitors it is at address 0xB0000.
                                                                                        Text mode memory takes two bytes for every "character" on screen. One is the ASCII code byte, the other the attribute byte. so the text "HeLlo" would be stored as:
                                                                                        0x000b8000: 'H', colour_for_H
                                                                                        0x000b8002: 'e', colour_for_e
                                                                                        0x000b8004: 'L', colour_for_L
                                                                                        0x000b8006: 'l', colour_for_l
                                                                                        0x000b8008: 'o', colour_for_o
                                                                                        
                                                                                        The attribute byte carries the foreground colour in its lowest 4 bits and the background color in its highest 3 bits. The interpretation of bit #7 depends on how you (or the BIOS) configured the hardware 
                                                                                        */
#include <common/types.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <syscalls.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <drivers/ata.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <multitasking.h>
#include <drivers/amd_am79c973.h>
#include <net/etherframe.h>
#include <net/arp.h>
#include <net/ipv4.h>


//#define GRAPHICSMODE



using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;
using namespace myos::net;
                                                                                        
void printf(char* str)
{
    /* Using the static keyword on a local variable changes its duration from automatic duration to static duration. This means the variable is now created at the start of the program, and destroyed at the end of the program (just like a global variable). As a result, the static variable will retain its value even after it goes out of scope! */
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;
    
    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x=0;
                y++;
                break;
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
                break;
        }
        
        if(x>=80)
        {
            x=0;
            y++;
        }
        
        if(y>=25)
        {
            for(y=0;y<25;y++)
            {
                for(x=0;x<80;x++)
                {
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
                }
            }
                
            x=0;
            y=0;
                
        }
        
    }
        
}


void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0]= hex[(key>>4) & 0xF];
    foo[1]= hex[key & 0xF];
    printf(foo);
}

void printfHex16(uint16_t key)
{
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}

void printfHex32(uint32_t key)
{
    printfHex((key >> 24) & 0xFF);
    printfHex((key >> 16) & 0xFF);
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}



class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    /* This function overrides the OnKeyDown in KeyboardEventHandler */
    void OnKeyDown(char c)          
    {
        char* foo = " ";
        foo[0]=c;
        printf(foo);
    }
};


class MouseToConsole : public MouseEventHandler
{
    int8_t x,y;
public:
    MouseToConsole()
    {
        uint16_t* VideoMemory = (uint16_t*) 0xB8000;
    
        //The mouse cursor is set initially at the center of the screen .
        x=40;
        y=12;
        
        //Wherever the cursor points , swap the foreground and background colors of that char .
        VideoMemory[80*y + x] = (VideoMemory[80*y + x] & 0x0F00) << 4 | (VideoMemory[80*y + x] & 0xF000) >> 4 | (VideoMemory[80*y + x] & 0x00FF) ;  
    }
    
    ~MouseToConsole(){}
    
    virtual void OnMouseMove(int xoffset, int yoffset)
    {
            static uint16_t* VideoMemory = (uint16_t*)0xB8000;
            VideoMemory[80*y +x] = (VideoMemory[80*y + x] & 0x0F00) << 4 | (VideoMemory[80*y + x] & 0xF000) >> 4 | (VideoMemory[80*y + x] & 0x00FF) ;
            
            x += xoffset;
            if(x>=80) x=79;
            if(x<0) x=0;
            
            y += yoffset;
            /* We make y -= buffer[2] because "delta Y", with down (toward the user) being negative. So if we move the mouse down we are actually increasing with respect to the video memory , so we subtract so that we get a +ve increase in video memory address. */
            if(y>=25) y=24;
            if(y<0) y=0;
            VideoMemory[80*y +x] = (VideoMemory[80*y + x] & 0x0F00) << 4 | (VideoMemory[80*y + x] & 0xF000) >> 4 | (VideoMemory[80*y + x] & 0x00FF) ;
  
    }
};


void sysprintf(char* str)
{
    asm("int $0x80" : : "a" (4), "b" (str)); //puts 4 in register a and str in register b , 4 is the syscall number
}

void taskA()
{
    while(true)
        sysprintf("A");
}

void taskB()
{
    while(true)
        sysprintf("B");
}


typedef void (*constructor)();                                      /*define constructor as a pointer to a function*/
extern "C" constructor start_ctors;                                 /*start_ctors is a variable of type constructor(pointer to a function)*/
extern "C" constructor end_ctors;                                   /*end_ctors is a variable of type constructor(pointer to a function)*/
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++) 
    {
        /* i is a variable of type pointer to a pointer to a function*/ /*It at first is pointing to the first pointer to a function element (start_ctors) in the array of pointers to functions(_init_array_) */
        (*i)();                              /* i is derefernced so it now is the function then it gets called*/
    }
}
 
 /*The compiler g++ mangles the name of the function kernelMain when it compiles so we have to write extern "C" to stop it from mangling the name*/
extern "C" void kernelMain(const void* multiboot_structure , uint32_t /*multiboot_magic*/)
{
   //printf("Hello From The Kernel \n");
   
    
    GlobalDescriptorTable gdt;
    printf("GDT initialized and loaded\n");
    
    /*
     memupper is a field at offset 8 in the Multiboot information structure and it indicates the amount upper memory in kilobytes.
     Lower memory starts at address 0, and upper memory starts at address 1 megabyte. 
     The maximum possible value for lower memory is 640 kilobytes. The value returned for upper memory is maximally the address of the first upper memory hole minus 1 megabyte.
     */
    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    
    size_t heap = 10*1024*1024;     // The start of the heap will be at 10 MB.
    
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);
    //The value of upper memory is in KB in memupper field so we multiply by KB.
    //The size of heap is all available size of upper memory - heap start address - 10 KB padding .

    printf("heap: 0x");
    printfHex((heap >> 24) & 0xFF);
    printfHex((heap >> 16) & 0xFF);
    printfHex((heap >> 8 ) & 0xFF);
    printfHex((heap      ) & 0xFF);

    void* allocated = memoryManager.malloc(1024);
    printf("\nallocated: 0x");
    printfHex(((size_t)allocated >> 24) & 0xFF);
    printfHex(((size_t)allocated >> 16) & 0xFF);
    printfHex(((size_t)allocated >> 8 ) & 0xFF);
    printfHex(((size_t)allocated      ) & 0xFF);
    printf("\n");
    
    TaskManager taskManager;
    
    /*
    Task task1(&gdt, taskA);
    Task task2(&gdt, taskB);
    taskManager.AddTask(&task1);
    taskManager.AddTask(&task2);
    */
    
    InterruptManager interrupts(0x20, &gdt, &taskManager);                    //pass base address for IRQs after remapping and pass address of GDT , this will set the idt and load it .
    printf("IDT initialized and loaded and tasks created \n");
    
    SyscallHandler syscalls(&interrupts, 0x80);
    
    #ifdef GRAPHICSMODE
        Desktop desktop(320,200, 0x00,0x00,0xA8);
    #endif
    
    DriverManager drvManager;
    printf("Driver Manager initialized ");
    
    #ifdef GRAPHICSMODE
            KeyboardDriver keyboard(&interrupts, &desktop);
    #else
            PrintfKeyboardEventHandler kbhandler;
            KeyboardDriver keyboard(&interrupts, &kbhandler);
    #endif
    drvManager.AddDriver(&keyboard);
    printf("Keyboard initialized\n");
    
   #ifdef GRAPHICSMODE
            MouseDriver mouse(&interrupts, &desktop);
   #else
            MouseToConsole mousehandler;
            MouseDriver mouse(&interrupts, &mousehandler);
   #endif 
    drvManager.AddDriver(&mouse);
    printf("Mouse initialized\n");
    
    PeripheralComponentInterconnectController PCIController;
    PCIController.SelectDrivers(&drvManager, &interrupts);
    
    #ifdef GRAPHICSMODE
            VideoGraphicsArray vga;
        #endif
        
    drvManager.ActivateAll();
    printf("All Drivers Activted\n");
    
     #ifdef GRAPHICSMODE
        vga.SetMode(320,200,8);
        Window win1(&desktop, 10,10,20,20, 0xA8,0x00,0x00);
        desktop.AddChild(&win1);
        Window win2(&desktop, 40,15,30,30, 0x00,0xA8,0x00);
        desktop.AddChild(&win2);
    #endif
        
  /*      
    //interrupt 14    
    printf("\nS-ATA primary master: ");
    AdvancedTechnologyAttachment ata0m(true, 0x1F0);
    ata0m.Identify();

    printf("\nS-ATA primary slave: ");
    AdvancedTechnologyAttachment ata0s(false, 0x1F0);
    ata0s.Identify();
    ata0s.Write28(0, (uint8_t*)"http://www.AlgorithMan.de", 25);
    ata0s.Flush();
    ata0s.Read28(0,25);
    

    //interrupt 15
    printf("\nS-ATA secondary master: ");
    AdvancedTechnologyAttachment ata1m(true, 0x170);
    ata1m.Identify();

    printf("\nS-ATA secondary slave: ");
    AdvancedTechnologyAttachment ata1s(false, 0x170);
    ata1s.Identify();

    // third: 0x1E8
    // fourth: 0x168
 */
  
  
  amd_am79c973* eth0 = (amd_am79c973*)(drvManager.drivers[2]);
  
  // IP 10.0.2.15   (Our virtual Box ip)
    uint8_t ip1 = 10, ip2 = 0, ip3 = 2, ip4 = 15;
    uint32_t ip_be = ((uint32_t)ip4 << 24)
                | ((uint32_t)ip3 << 16)
                | ((uint32_t)ip2 << 8)
                | (uint32_t)ip1;
                
    eth0->SetIPAddress(ip_be);
   
   EtherFrameProvider etherframe(eth0);
   
   AddressResolutionProtocol arp(&etherframe);

    // IP 10.0.2.2  (virtual Box gateway ip )    
    uint8_t gip1 = 10, gip2 = 0, gip3 = 2, gip4 = 2;
    uint32_t gip_be = ((uint32_t)gip4 << 24)
                   | ((uint32_t)gip3 << 16)
                   | ((uint32_t)gip2 << 8)
                   | (uint32_t)gip1;
                   
    uint8_t subnet1 = 255, subnet2 = 255, subnet3 = 255, subnet4 = 0;
    uint32_t subnet_be = ((uint32_t)subnet4 << 24)
                   | ((uint32_t)subnet3 << 16)
                   | ((uint32_t)subnet2 << 8)
                   | (uint32_t)subnet1;
  
                   
    InternetProtocolProvider ipv4(&etherframe, &arp, gip_be, subnet_be);
   
   
   //etherframe.Send(0xFFFFFFFFFFFF, 0x0608, (uint8_t*)"FOO", 3);  //ethertype is in Big Endian -> original : 0x0806
   // eth0->Send((uint8_t*)"Hello Network", 13);


        
    interrupts.Activate();
    printf("Interrupts Activated\n");
    
    printf("Enter text and use the mouse : \n");
    
    printf("\n\n\n\n\n\n\n\n");
    //arp.Resolve(gip_be);
    ipv4.Send(gip_be, 0x0008, (uint8_t*) "foobar", 6);
    
    
        while(1)
    {
        #ifdef GRAPHICSMODE
            desktop.Draw(&vga);
        #endif
    }
}
