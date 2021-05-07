#include <drivers/keyboard.h>
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

KeyboardEventHandler::KeyboardEventHandler()
{
}

void KeyboardEventHandler::OnKeyDown(char)
{
}

void KeyboardEventHandler::OnKeyUp(char)
{
}

KeyboardDriver::KeyboardDriver(InterruptManager* manager, KeyboardEventHandler* handler)
:InterruptHandler(manager, 0x21),
dataport(0x60),
commandport(0x64)
{
    this->handler=handler;
}

KeyboardDriver::~KeyboardDriver()
{
}

void printf(char*);
void printfHex(uint8_t);

/*
Keyboard Encoder :
    port [0x60] ( Read ) : Read Input Buffer
    port [0x60] ( Write ): Send Command

Onboard Keyboard Controller :
    port [0x64] ( Read ): Status Register
    port [0x64] ( Write ): Send Command
    Bit 0 [Output Buffer Status] (0: Output buffer empty, don't read yet) (1: Output buffer full, please read me).
    Bit 1 [Input Buffer Status] (0: Input buffer empty, can be written) (1: Input buffer full, don't write yet)
    
Before writing to port 0x60 or 0x64, the input buffer must be empty. (Port [0x64] .Bit [1] == 0)
Status of the input buffer (KBC <- CPU): 0 = empty (writing to 0x60 or 0x64 possible); 1 = full.
Before reading port 0x60, the output buffer must be full (port [0x64] .Bit [0] == 1)
 */

void KeyboardDriver::Activate()
{
   while(commandport.Read() & 0x1)
    {
        dataport.Read();
/*
As long as bit 0 is set in the status register (buffer full), read bytes from the data register. This helps if any keys were pressed before the driver was initialized, as the KBC then sends no further characters until the buffer has been emptied (read out). This often occurs in connection with GRUB, for example, when the user holds the key down for too long.

//clear the keyboard buffer
while (inb (0x64) & 0x1) { inb (0x60);}
*/
    }
    
/*
 Keyboard Controller Commands :
    0xAE : Enable Keyboard
    0x20 : Read command byte , after that we read the status from data port
    0x60 : Write command byte , after that we change the state of the data port

    Keyboard encoder command [0xF4] : Enable keyboard .
*/
    commandport.Write(0xae);
    commandport.Write(0x20);
    uint8_t status = (dataport.Read() | 1) & ~0x10;
/*    Setting Bit 0 : Activates the activation of IRQ1 if keyboard data such as scan codes are present in the buffer (Bit1 set at 64h) .
      Clearing Bit 4 : Enables keyboard */ 
    commandport.Write(0x60);
    dataport.Write(status);
    dataport.Write(0xf4); 
}

uint32_t KeyboardDriver::HandleInterrupt(uint32_t esp)
{
//The 8th bit is set to 1 if key is released and cleared to 0 if key is pressed , so when you a press a key and then release it you get two interrupts from the keyboard one is for pressing and the second is for releasing .
 
    uint8_t key = dataport.Read();
    
    if(handler==0)
    {
        return esp;
    }
    
    static bool Shift = false;
    
    /* by default OnKeyDown() does nothing as defined in the KeyboardEventHandler class . But we can define another class derived from the KeyboardEventHandler class and implement in it OnKeyDown() that does anything when key is pressed . For example : we define PrintfKeyboardEventHandler in kernel.cpp which is derived from KeyboardEventHandler and the OnKeyDown() function defined in it will print the key to the screen . */
       
        switch(key)
        {
            
            case 0x29: if(Shift) handler->OnKeyDown('~'); else handler->OnKeyDown('`'); break;
            case 0x02: if(Shift) handler->OnKeyDown('!'); else handler->OnKeyDown('1'); break;
            case 0x03: if(Shift) handler->OnKeyDown('@'); else handler->OnKeyDown('2'); break;
            case 0x04: if(Shift) handler->OnKeyDown('#'); else handler->OnKeyDown('3'); break;
            case 0x05: if(Shift) handler->OnKeyDown('$'); else handler->OnKeyDown('4'); break;
            case 0x06: if(Shift) handler->OnKeyDown('%'); else handler->OnKeyDown('5'); break;
            case 0x07: if(Shift) handler->OnKeyDown('^'); else handler->OnKeyDown('6'); break;
            case 0x08: if(Shift) handler->OnKeyDown('&'); else handler->OnKeyDown('7'); break;
            case 0x09: if(Shift) handler->OnKeyDown('*'); else handler->OnKeyDown('8'); break;
            case 0x0A: if(Shift) handler->OnKeyDown('('); else handler->OnKeyDown('9'); break;
            case 0x0B: if(Shift) handler->OnKeyDown(')'); else handler->OnKeyDown('0'); break;
            case 0x0C: if(Shift) handler->OnKeyDown('_'); else handler->OnKeyDown('-'); break;
            case 0x0D: if(Shift) handler->OnKeyDown('+'); else handler->OnKeyDown('='); break;
            
            case 0x10: if(Shift) handler->OnKeyDown('Q'); else handler->OnKeyDown('q'); break;
            case 0x11: if(Shift) handler->OnKeyDown('W'); else handler->OnKeyDown('w'); break;
            case 0x12: if(Shift) handler->OnKeyDown('E'); else handler->OnKeyDown('e'); break;
            case 0x13: if(Shift) handler->OnKeyDown('R'); else handler->OnKeyDown('r'); break;
            case 0x14: if(Shift) handler->OnKeyDown('T'); else handler->OnKeyDown('t'); break;
            case 0x15: if(Shift) handler->OnKeyDown('Y'); else handler->OnKeyDown('y'); break;
            case 0x16: if(Shift) handler->OnKeyDown('U'); else handler->OnKeyDown('u'); break;
            case 0x17: if(Shift) handler->OnKeyDown('I'); else handler->OnKeyDown('i'); break;
            case 0x18: if(Shift) handler->OnKeyDown('O'); else handler->OnKeyDown('o'); break;
            case 0x19: if(Shift) handler->OnKeyDown('P'); else handler->OnKeyDown('p'); break;
            case 0x1A: if(Shift) handler->OnKeyDown('{'); else handler->OnKeyDown('['); break;
            case 0x1B: if(Shift) handler->OnKeyDown('}'); else handler->OnKeyDown(']'); break;
            case 0x2B: if(Shift) handler->OnKeyDown('|'); else handler->OnKeyDown('\\'); break;
            
            case 0x1E: if(Shift) handler->OnKeyDown('A'); else handler->OnKeyDown('a'); break;
            case 0x1F: if(Shift) handler->OnKeyDown('S'); else handler->OnKeyDown('s'); break;
            case 0x20: if(Shift) handler->OnKeyDown('D'); else handler->OnKeyDown('d'); break;
            case 0x21: if(Shift) handler->OnKeyDown('F'); else handler->OnKeyDown('f'); break;
            case 0x22: if(Shift) handler->OnKeyDown('G'); else handler->OnKeyDown('g'); break;
            case 0x23: if(Shift) handler->OnKeyDown('H'); else handler->OnKeyDown('h'); break;
            case 0x24: if(Shift) handler->OnKeyDown('J'); else handler->OnKeyDown('j'); break;
            case 0x25: if(Shift) handler->OnKeyDown('K'); else handler->OnKeyDown('k'); break;
            case 0x26: if(Shift) handler->OnKeyDown('L'); else handler->OnKeyDown('l'); break;
            case 0x27: if(Shift) handler->OnKeyDown(':'); else handler->OnKeyDown(';'); break;
            case 0x28: if(Shift) handler->OnKeyDown('\"'); else handler->OnKeyDown('\''); break;
            
            
            case 0x2C: if(Shift) handler->OnKeyDown('Z'); else handler->OnKeyDown('z'); break;
            case 0x2D: if(Shift) handler->OnKeyDown('X'); else handler->OnKeyDown('x'); break;
            case 0x2E: if(Shift) handler->OnKeyDown('C'); else handler->OnKeyDown('c'); break;
            case 0x2F: if(Shift) handler->OnKeyDown('V'); else handler->OnKeyDown('v'); break;
            case 0x30: if(Shift) handler->OnKeyDown('B'); else handler->OnKeyDown('b'); break;
            case 0x31: if(Shift) handler->OnKeyDown('N'); else handler->OnKeyDown('n'); break;
            case 0x32: if(Shift) handler->OnKeyDown('M'); else handler->OnKeyDown('m'); break;
            case 0x33: if(Shift) handler->OnKeyDown('<'); else handler->OnKeyDown(','); break;
            case 0x34: if(Shift) handler->OnKeyDown('>'); else handler->OnKeyDown('.'); break;
            case 0x35: if(Shift) handler->OnKeyDown('?'); else handler->OnKeyDown('/'); break;
            
            //numpad
            case 0x52: handler->OnKeyDown('0'); break;
            case 0x4F: handler->OnKeyDown('1'); break;
            case 0x50: handler->OnKeyDown('2'); break;
            case 0x51: handler->OnKeyDown('3'); break;
            case 0x4B: handler->OnKeyDown('4'); break;
            case 0x4C: handler->OnKeyDown('5'); break;
            case 0x4D: handler->OnKeyDown('6'); break;
            case 0x47: handler->OnKeyDown('7'); break;
            case 0x48: handler->OnKeyDown('8'); break;
            case 0x49: handler->OnKeyDown('9'); break;
            case 0x37: handler->OnKeyDown('*'); break;
            case 0x4A: handler->OnKeyDown('-'); break;
            case 0x4E: handler->OnKeyDown('+'); break;
            case 0x53: handler->OnKeyDown('.'); break;
            
            case 0x1C: handler->OnKeyDown('\n'); break;
            case 0x39: handler->OnKeyDown(' '); break;
            case 0x2A: case 0x36: Shift = true; break;
            case 0xAA: case 0xB6: Shift = false; break;
            
            case 0x45: break; //numlock ignored
        
            default:
            {
                if(key < 0x80) //ignore interrupts triggered by release of the key
   
                {
                    printf("KEYBOARD 0x");
                    printfHex(key);
                    break;
                }
            }
        }
    
    return esp;
}
