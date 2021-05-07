#ifndef __MYOS__HARDWARECOMMUNICATION__PCI_H
#define __MYOS__HARDWARECOMMUNICATION__PCI_H

#include <hardwarecommunication/port.h>
#include <drivers/driver.h>
#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <memorymanagement.h>

namespace myos
{
    namespace hardwarecommunication
    {
        enum BaseAddressRegisterType
        {
            MemoryMapping = 0,
            InputOutput = 1
        };



        class BaseAddressRegister
        {
        public:
            bool prefetchable;
            myos::common::uint8_t* address;             //the addrees itself is 32 bits but it may be pointing to an 8-bit register
            myos::common::uint32_t size;
            BaseAddressRegisterType type;
        };

        
        //PeripheralComponentInterconnectDeviceDescriptor is descriptor for configuration address space http://www.lowlevel.eu/wiki/Peripheral_Component_Interconnect#Aufbau_des_Konfigurations-Adressraum
         class PeripheralComponentInterconnectDeviceDescriptor
        {
          public:
            myos::common::uint32_t portBase;
            myos::common::uint32_t interrupt;

            myos::common::uint16_t bus;
            myos::common::uint16_t device;
            myos::common::uint16_t function;

            myos::common::uint16_t vendor_id;
            myos::common::uint16_t device_id;

            myos::common::uint8_t class_id;
            myos::common::uint8_t subclass_id;
            myos::common::uint8_t interface_id;

            myos::common::uint8_t revision;

            PeripheralComponentInterconnectDeviceDescriptor();
            ~PeripheralComponentInterconnectDeviceDescriptor();

        };


        class PeripheralComponentInterconnectController
        {
            Port32Bit dataPort;
            Port32Bit commandPort;

          public:
            PeripheralComponentInterconnectController();
            ~PeripheralComponentInterconnectController();
            //Read() reads a Dword from a PCI configuration register .
             myos::common::uint32_t Read(myos::common::uint16_t bus, myos::common::uint16_t device, myos::common::uint16_t function, myos::common::uint32_t registeroffset);
            //Write() writes a Dword to a PCI configuration register .
             void Write(myos::common::uint16_t bus, myos::common::uint16_t device, myos::common::uint16_t function, myos::common::uint32_t registeroffset, myos::common::uint32_t value);
            bool DeviceHasFunctions(myos::common::uint16_t bus, myos::common::uint16_t device);

            void SelectDrivers(myos::drivers::DriverManager* driverManager, myos::hardwarecommunication::InterruptManager* interrupts);
            PeripheralComponentInterconnectDeviceDescriptor GetDeviceDescriptor(myos::common::uint16_t bus, myos::common::uint16_t device, myos::common::uint16_t function);
            myos::drivers::Driver* GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, myos::hardwarecommunication::InterruptManager* interrupts);
            BaseAddressRegister GetBaseAddressRegister(myos::common::uint16_t bus, myos::common::uint16_t device, myos::common::uint16_t function, myos::common::uint16_t bar);
            //bar variable refers to the base address register number 0-5 in case of header type 0 or 0-1 in case of header type 1
        };

    }
}

#endif
