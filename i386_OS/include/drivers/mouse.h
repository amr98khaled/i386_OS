#ifndef __MYOS__DRIVERS__MOUSE_H
#define __MYOS__DRIVERS__MOUSE_H

#include <common/types.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>
#include <hardwarecommunication/interrupts.h>

namespace myos
{
    namespace drivers
    {
        /* MouseEventHandler is a base class for mouse handlers from which various mouse handlers classes will be derived . For example the default handler does nothing when the mouse moves or a button is pressed but you can derive a class that moves the cursor when the mouse moves , ...etc  */
        class MouseEventHandler
        {
        public:
            MouseEventHandler();
            virtual void OnActivate();
            virtual void OnMouseDown(myos::common::uint8_t button); //handles the press of mouse buttons
            virtual void OnMouseUp(myos::common::uint8_t button);  // handles the release of mouse buttons
            virtual void OnMouseMove(int x, int y);
        };

        class MouseDriver: public myos::hardwarecommunication::InterruptHandler, public Driver
        {
        myos::hardwarecommunication::Port8Bit dataport;
        myos::hardwarecommunication::Port8Bit commandport;
        myos::common::uint8_t buffer[3];
        /*
        Byte 1 : Y overflow | X overflow | Y sign bit | X sign bit | Reserved (1) | Middle button pressed | Right button pressed | Left button pressed
        Byte 2 : X Movement since the last data packet. "delta X" value -- that is, it measures horizontal mouse movement, with left being negative.
        Byte 3 : Y Movement since the last data packet, "delta Y", with down (toward the user) being negative.

        The mouse triggers 3 interrupts , one for each byte .
        */
        
        myos::common::uint8_t offset;
        myos::common::uint8_t buttons;
        
        MouseEventHandler* handler;
        
        public:
            MouseDriver(myos::hardwarecommunication::InterruptManager* manager, MouseEventHandler* handler);
            ~MouseDriver();
            virtual myos::common::uint32_t HandleInterrupt(myos::common::uint32_t esp);
            /* HandleInterrupt() in InterruptHandler class only just returned the esp . But here in the MouseDriver class it will handle the mouse movement and buttons pressed */
            virtual void Activate();
            /* Activate() in the Driver class did nothing but here it will Activate the mouse */
        };
    }
    
}
#endif
