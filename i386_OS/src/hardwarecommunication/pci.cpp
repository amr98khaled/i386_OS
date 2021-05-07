/*
 -The configuration address space is read and written to, in the case of PCs, 32-bit with the aid of an address I/O port (0x0CF8) and a data I/O port (0x0CFC).
-Structure of the address I/O port :
31 | Enable bit
30-24 | Reserved
23-16 | Bus number
15-11 | Device number
10-8 | Function number
7-0 | Register number , bit 0 and bit 1 are 0
 */

#include <hardwarecommunication/pci.h>
#include <drivers/amd_am79c973.h>
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

PeripheralComponentInterconnectDeviceDescriptor::PeripheralComponentInterconnectDeviceDescriptor()
{
}

PeripheralComponentInterconnectDeviceDescriptor::~PeripheralComponentInterconnectDeviceDescriptor()
{
}


PeripheralComponentInterconnectController::PeripheralComponentInterconnectController()
: dataPort(0xCFC),
  commandPort(0xCF8)
{
}

PeripheralComponentInterconnectController::~PeripheralComponentInterconnectController()
{
}

uint32_t PeripheralComponentInterconnectController::Read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset)
{
    //id is Structure of the address I/O port
    uint32_t id =
        0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (registeroffset & 0xFC);
    commandPort.Write(id);
    uint32_t result = dataPort.Read();
    return result >> (8* (registeroffset % 4));
    // x % y = (x & (y − 1)) , so (registeroffset % 4) = (registeroffset & 3) .
}

void PeripheralComponentInterconnectController::Write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value)
{
    uint32_t id =
        0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (registeroffset & 0xFC);
    commandPort.Write(id);
    dataPort.Write(value); 
}

bool PeripheralComponentInterconnectController::DeviceHasFunctions(common::uint16_t bus, common::uint16_t device)
{
    return Read(bus, device, 0, 0x0E) & (1<<7);
    /*
     We check to see if the 7th bit in Header type register (0x0E) is set or not , if set then it has multiple functions
     if( (headerType & 0x80) != 0)
     */
}

void printf(char* str);
void printfHex(uint8_t);

void PeripheralComponentInterconnectController::SelectDrivers(DriverManager* driverManager, myos::hardwarecommunication::InterruptManager* interrupts)
{
    for(int bus = 0; bus < 8; bus++)
    {
        for(int device = 0; device < 32; device++)
        {
            int numFunctions = DeviceHasFunctions(bus, device) ? 8 : 1;
            for(int function = 0; function < numFunctions; function++)
            {
                PeripheralComponentInterconnectDeviceDescriptor dev = GetDeviceDescriptor(bus, device, function);

                if(dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF)
                    continue;
                /*
                 -The vendor IDs are managed by PCI-SIG and assigned to the manufacturers. The value 0xffff is reserved for the case that there is no device at this point. The value 0x0001 comes when the device is present but  not yet ready to react. The value 0x0000 is invalid (similar to 0xffff). All other values ​​can be assigned as valid vendor IDs.
                 A device can have functions that aren't consecutive like function 1 & 5 and not 2,3,4 so we must not break from the for loop so that we see all the functions of the device.
                 */
                
            for(int barNum = 0; barNum < 6; barNum++)
                {
                    BaseAddressRegister bar = GetBaseAddressRegister(bus, device, function, barNum);
                    if(bar.address && (bar.type == InputOutput))
                        dev.portBase = (uint32_t)bar.address;
                }

                 Driver* driver = GetDriver(dev, interrupts);
                    if(driver != 0)
                        driverManager->AddDriver(driver);

                
                printf("PCI BUS ");
                printfHex(bus & 0xFF);

                printf(", DEVICE ");
                printfHex(device & 0xFF);

                printf(", FUNCTION ");
                printfHex(function & 0xFF);

                printf(" = VENDOR ");
                printfHex((dev.vendor_id & 0xFF00) >> 8);
                printfHex(dev.vendor_id & 0xFF);
                printf(", DEVICE ");
                printfHex((dev.device_id & 0xFF00) >> 8);
                printfHex(dev.device_id & 0xFF);
                printf("\n");
            }
        }
    }
}


BaseAddressRegister PeripheralComponentInterconnectController::GetBaseAddressRegister(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar)
{
    BaseAddressRegister result;
    /*
        // only types 0x00 (normal devices) and 0x01 (PCI-to-PCI bridges) are supported:
        if (headerType> = 0x02)
        {printf ("ERROR: unsupported header type found! \ n"); return; }

        // 6 BARs for type 0x00 and 2 BARs for type 0x01:
        const uint max_bars = 6 - (headerType * 4);
     */
    
    uint32_t headertype = Read(bus, device, function, 0x0E) & 0x7F;
    int maxBARs = 6 - (4*headertype);
    if(bar >= maxBARs)
        return result;
    
    /* Determine the offset of the current BAR:
         const uint barOffset = 0x010 + (bar * 4);
       bar adresses begin at register 0x010
    */

        uint32_t bar_value = Read(bus, device, function, 0x10 + 4*bar);  //read the base address register number 'bar'
        result.type = (bar_value & 0x1) ? InputOutput : MemoryMapping;
        /*
            Bit 0 signals with its value 0 that the BAR concerned is a memory resource.
            Bit 0 signals with its value 1 that the BAR concerned is an IO resource.
        */
        
         uint32_t temp;



    if(result.type == MemoryMapping)
    {

        switch((bar_value >> 1) & 0x3)
        {

            case 0: // 32 Bit Mode
            case 1: // 20 Bit Mode
            case 2: // 64 Bit Mode
                break;
        }

    }
    else // InputOutput
    {
        result.address = (uint8_t*)(bar_value & ~0x3);
        result.prefetchable = false;
    }


    return result;
}


Driver* PeripheralComponentInterconnectController::GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, InterruptManager* interrupts)
{
    Driver* driver = 0;
    
    switch(dev.vendor_id)
    {
        case 0x1022: // AMD
            switch(dev.device_id)
            {
                case 0x2000: // am79c973
                   printf("AMD am79c973 ");
                   driver = (amd_am79c973*)MemoryManager::activeMemoryManager->malloc(sizeof(amd_am79c973));
                    if(driver != 0)
                        new (driver) amd_am79c973(&dev, interrupts);
                    else
                        printf("instantiation failed");
                    return driver;
                    break;
            }
            break;

        case 0x8086: // Intel
            break;
    }


    switch(dev.class_id)
    {
        case 0x03: // graphics
            switch(dev.subclass_id)
            {
                case 0x00: // VGA
                    printf("VGA ");
                    break;
            }
            break;
    }


    return driver;
}



PeripheralComponentInterconnectDeviceDescriptor PeripheralComponentInterconnectController::GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function)
{
    PeripheralComponentInterconnectDeviceDescriptor result;

    result.bus = bus;
    result.device = device;
    result.function = function;

    result.vendor_id = Read(bus, device, function, 0x00);
    result.device_id = Read(bus, device, function, 0x02);

    result.class_id = Read(bus, device, function, 0x0b);
    result.subclass_id = Read(bus, device, function, 0x0a);
    result.interface_id = Read(bus, device, function, 0x09);

    result.revision = Read(bus, device, function, 0x08);
    result.interrupt = Read(bus, device, function, 0x3c);

    return result;
}
