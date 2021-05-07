#ifndef __MYOS__DRIVERS__KEYBOARD_H
#define __MYOS__DRIVERS__KEYBOARD_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <drivers/driver.h>
#include <hardwarecommunication/port.h>

namespace myos
{
    namespace drivers
    {
        /* KeyboardEventHandler is a base class for keyboard handlers from which various keyboard handlers classes will be derived . For example the default handler does nothing when a key is pressed but you can derive a class that prints to the screen when a key is pressed , you can derive another class that moves a player in game when a key is pressed ..etc  */
        class KeyboardEventHandler
        {
        public:
            KeyboardEventHandler();
            
            virtual void OnKeyDown(char);
            virtual void OnKeyUp(char);
        };


        class KeyboardDriver : public myos::hardwarecommunication::InterruptHandler , public Driver
        {
        myos::hardwarecommunication::Port8Bit dataport;
        myos::hardwarecommunication::Port8Bit commandport;
        KeyboardEventHandler* handler;
        
        public:
            KeyboardDriver(myos::hardwarecommunication::InterruptManager* manager, KeyboardEventHandler* handler);
            ~KeyboardDriver();
            virtual myos::common::uint32_t HandleInterrupt(myos::common::uint32_t esp);
            /* HandleInterrupt() in InterruptHandler class only just returned the esp . But here in the KeyboardDriver class it will handle the key presses and releases */
            virtual void Activate();
            /* Activate() in the Driver class did nothing but here it will Activate the keyboard */
        };
    }
    
}
#endif
