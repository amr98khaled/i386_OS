#ifndef __MYOS__HARDWARECOMMUNICATION__PORT_H
#define __MYOS__HARDWARECOMMUNICATION__PORT_H

#include <common/types.h>

namespace myos
{
    namespace hardwarecommunication
    {


        /*
        NOTES
        1-protected : members can be used by objects of the derived classes but not by objects of the base class.
        2-virtual : if the base class and the derived class both have the same function [same name, same input parameters, same return type], then this function is overriden . So to make sure that when calling a member function from an object of the derived class , that this function called is the function that belongs to the derived class not to the base class , we use the virtual keyword to tell the compiler we want to resolve to the most derived class.
        3-static : 1.static member functions can be called without creating an object for this class . 2.Unlike normal member variables ,ststaic member variables are shared by all objects of the class.
                    Although you can access static members through objects of the class, it turns out that static members exist even if no objects of the class have been instantiated! Much like global variables, they are created when the   program starts, and destroyed when the program ends.Consequently, it is better to think of static members as belonging to the class itself, not to the objects of the class.
                    If the class is defined in a .h file, the static member definition is usually placed in the associated code file for the class (e.g. Something.cpp). If the class is defined in a .cpp file, the static member definition is usually placed directly underneath the class. Do not put the static member definition in a header file (much like a global variable, if that header file gets included more than once, you’ll end up with multiple definitions, which will cause a compile error).
        4-inline : when code is compiled , a call to a function is replaced with the function's code rather than calling the function which improves performance .
        5-For a better understanding for inline assembly visit : https://gcc.gnu.org/onlinedocs/gcc/Using-Assembly-Language-with-C.html
        */

        class Port
        {
        protected:
            Port(myos::common::uint16_t portnumber);      //Port number is a 16-bit number
            // FIXME: Must be virtual (currently isnt because the kernel has no memory management yet)
            ~Port();
            myos::common::uint16_t portnumber;
        };

        class Port8Bit: public Port
        {
        public:
            Port8Bit(myos::common::uint16_t portnumber);
            ~Port8Bit();
            virtual myos::common::uint8_t Read();
            virtual void Write(myos::common::uint8_t data);
            
        protected:
            static inline myos::common::uint8_t Read8(myos::common::uint16_t _port)
            {
                myos::common::uint8_t result;
                __asm__ volatile("inb %1, %0" : "=a" (result) : "Nd" (_port));         
        /* The in instruction (returning a byte) can either take an immediate 8 bit value as a port number, or a port specified in the dx register. Nd--> d:The d register & N:Unsigned 8-bit integer constant */
                return result;
            }
            
            static inline void Write8(myos::common::uint16_t _port, myos::common::uint8_t _data)
            {
                __asm__ volatile("outb %0, %1" : : "a" (_data), "Nd" (_port));
            }
            
        };


        class Port8BitSlow: public Port8Bit
        {
        public:
            Port8BitSlow(myos::common::uint16_t portnumber);
            ~Port8BitSlow();
            virtual void Write(myos::common::uint8_t data);
            
        protected:
            static inline void Write8Slow(myos::common::uint16_t _port, myos::common::uint8_t _data)
            {
                __asm__ volatile("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a" (_data), "Nd" (_port));
                /* By adding garbage instructions this write method is slower. jump 1f : jump forward to first numeric label "1" defined after this instruction --> jumps to numeric label 1: */
            }
            
        };


        class Port16Bit: public Port
        {
        public:
            Port16Bit(myos::common::uint16_t portnumber);
            ~Port16Bit();
            virtual myos::common::uint16_t Read();
            virtual void Write(myos::common::uint16_t data);
            
        protected:
            static inline myos::common::uint16_t Read16(myos::common::uint16_t _port)
            {
                myos::common::uint16_t result;
                __asm__ volatile("inw %1, %0" : "=a" (result): "Nd" (_port));         
        /* The in instruction (returning a byte) can either take an immediate 8 bit value as a port number, or a port specified in the dx register. Nd--> d:The d register & N:Unsigned 8-bit integer constant */
                return result;
            }
            
            static inline void Write16(myos::common::uint16_t _port, myos::common::uint16_t _data)
            {
                __asm__ volatile("outw %0, %1" : : "a" (_data), "Nd" (_port));
            }
            
        };



        class Port32Bit: public Port
        {
        public:
            Port32Bit(myos::common::uint16_t portnumber);
            ~Port32Bit();
            virtual myos::common::uint32_t Read();
            virtual void Write(myos::common::uint32_t data);
            
        protected:
            static inline myos::common::uint32_t Read32(myos::common::uint16_t _port)
            {
                myos::common::uint32_t result;
                __asm__ volatile("inl %1, %0" : "=a" (result) : "Nd" (_port));         
        /* The in instruction (returning a byte) can either take an immediate 8 bit value as a port number, or a port specified in the dx register. Nd--> d:The d register & N:Unsigned 8-bit integer constant */
                return result;
            }
            
            static inline void Write32(myos::common::uint16_t _port, myos::common::uint32_t _data)
            {
                __asm__ volatile("outl %0, %1" : : "a" (_data), "Nd" (_port));
            }
            
        };

    }
    
}

#endif
