/*
The mouse itself and the sending of the IRQ12 must first be activated by the keyboard controller. The keyboard controller can be addressed via ports 0x60 and 0x64. Port 0x60 serves as a data buffer, port 0x64 as status and command register:

Reading 0x60 : supplies the content of the output buffer.
Writing 0x60 : after sending command 0x60 to commandport 0x64, data is written into the input buffer.
Reading 0x64 supplies the status byte of the keyboard controller.
Writing to 0x64 sends a command to the keyboard controller.

Status Register :
-Bit 0 : (clear=0){There is no data in the output buffer.} , (set=1){There is data in the output buffer, reading from port 0x60 is therefore permitted.}
-Bit 1 : (clear=0){The input buffer is empty. Commands may be sent via ports 0x60 and 0x64.} , (set=1){There is still data in the input buffer. The keyboard controller is currently not accepting any commands.}

Commands to the keyboard controller:
0xA8 | Activate mouse
0xA7 | Deactivate mouse
0x20 | Read command byte
0x60 | Write command byte
0xD4 | send the next command to the mouse instead of the keyboard

After 0xD4 is sent to the keyboard controller, it forwards the following commands to the mouse instead of the keyboard , these commands are written to the dataport 0x60:
0xF4 | Tell the mouse to send data to the CPU , Enable Packet streaming when mouse is moved or clicked .
0xF5 | Tell the mouse not to send any data to the CPU
0xF6 | Reset mouse settings to default settings
-The mouse responds to all of these commands by placing the value 0xFA (called ACK - Acknowledge) in the output buffer . In the following code the ACK byte is not checked .

-To read the command byte you first have to send the command 0x20 to the keyboard controller, whereupon the keyboard controller places the byte in the output buffer (dataport 0x60).
-To write the command byte, you must first send the command 0x60 to the keyboard controller and then place the command byte in the input buffer(dataport 0x60).

-All output to port 0x60 or 0x64 must be preceded by waiting for bit 1 (value=2) of port 0x64 to become clear. Similarly, bytes cannot be read from port 0x60 until bit 0 (value=1) of port 0x64 is set.

-Sending a command or data byte to the mouse (to port 0x60) must be preceded by sending a 0xD4 byte to port 0x64 (with appropriate waits on port 0x64, bit 1, before sending each output byte). Note: this 0xD4 byte does not generate any ACK, from either the keyboard or mouse.

-It is required to wait until the mouse sends back the 0xFA acknowledgement byte after each command or data byte before sending the next byte (Note: reset commands might not be ACK'ed -- wait for the 0xAA after a reset). A few commands require an additional data byte, and both bytes will generate an ACK.
 */

#include <drivers/mouse.h>
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

void printf(char*);

MouseEventHandler::MouseEventHandler()
{
}

void MouseEventHandler::OnActivate()
{
}

void MouseEventHandler::OnMouseDown(uint8_t button)
{
}

void MouseEventHandler::OnMouseUp(uint8_t button)
{
}

void MouseEventHandler::OnMouseMove(int x, int y)
{
}





MouseDriver::MouseDriver(InterruptManager* manager, MouseEventHandler* handler)
:InterruptHandler(manager,0x2C),
dataport(0x60),
commandport(0x64)
{
    this->handler=handler;
}
    
MouseDriver::~MouseDriver()
{
}

void MouseDriver::Activate()
{
   offset=0;
   buttons=0;
   
   if(handler!=0)
   {
     handler->OnActivate();   
     
   }
   
    //We are supposed to empty the data buffer first like we did with the keyboard  : while (inb (0x64) & 0x01) { inb (0x60); }
    
    commandport.Write(0xA8);
    commandport.Write(0x20); //read controller command byte
     
//Setting Bit 1 : Enables IRQ12 , Activates the activation of IRQ12 if mouse data, such as the MouseDataPacket, are present in the buffer. On some systems you also have to clear bit number 5 (value=0x20, Disable Mouse Clock) .
    uint8_t status = dataport.Read() | 2 ; 

    commandport.Write(0x60);    // set controller command byte
    dataport.Write(status);                       
    
    commandport.Write(0xD4);
    dataport.Write(0xF4);
    dataport.Read();
}

uint32_t MouseDriver::HandleInterrupt(uint32_t esp)
{
    uint8_t status = commandport.Read();
    //In that status byte from commandport 0x64, bit number 5 (value=0x20), indicates that this next byte came from the mouse, if the bit is set.
    if(!(status & 0x20))
        return esp;
    
    buffer[offset] = dataport.Read();
    
    if(handler==0)
        return esp;
    
    offset = (offset + 1) % 3;
    
    if(offset == 0)         // we have received the 3 bytes from the mouse through 3 interrupts
    {
        if(buffer[1] !=0 || buffer[2]!=0)       //if values of x or y have been updated
        {
            handler->OnMouseMove((int8_t)buffer[1], -((int8_t)buffer[2]));
            /* By default OnMouseMove function defined in MouseEventHandler class does nothing but the OnMouseMove() defined in MouseToConsole class which is a derived class from the MouseEventHandler class , moves the cursor and displays it to the screen  */
        }
        
        for(uint8_t i=0;i<3;i++) // We check the state of the 3 buttons of the mouse
        {
            if( (buffer[0] & (0x1<<i)) != (buttons & (0x1<<i)) ) //if the current state of the buttons is not equal to the previous state of the buttons , then the button must have been pressed or released
            {
              /* The below if condition is true if the previous state of the button was set to 1 (it was pressed) , so now it must be released as the above if condition was true because the state of the button changed .*/
                if(buttons & (0x1<<i)) 
                {
                    handler->OnMouseUp(i+1);    //handle the release of the button
                }
                else
                {
                    handler->OnMouseDown(i+1);  //else the button is pressed and handle the press
                }
            }
        }
        
        buttons = buffer[0];                    // button stores the current state of the buttons so that it will be compared to the next state of buttons when a mouse interrupt occurs
        
    }
    return esp;
}
