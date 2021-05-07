#include <drivers/ata.h>

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;


void printf(char* str);
void printfHex(uint8_t);

AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(bool master, common::uint16_t portBase)
:   dataPort(portBase),
    errorPort(portBase + 0x1),
    sectorCountPort(portBase + 0x2),
    lbaLowPort(portBase + 0x3),
    lbaMidPort(portBase + 0x4),
    lbaHiPort(portBase + 0x5),
    devicePort(portBase + 0x6),
    commandPort(portBase + 0x7),
    controlPort(portBase + 0x206)
{
    this->master = master;
}

AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment()
{
}

void AdvancedTechnologyAttachment::Identify()
{
    devicePort.Write(master ? 0xA0 : 0xB0);
    
    // This clears HOB bit. HOB : Set this to read back the High Order Byte of the last LBA48 value sent to an IO port.
       controlPort.Write(0);
       
   // Floating Bus check : First you select the master, then read in the value of the status register and then compare it with 0xFF (is an invalid status value).  
       devicePort.Write(0xA0);
       uint8_t status = commandPort.Read();
        if(status == 0xFF)
            return;
        
    devicePort.Write(master ? 0xA0 : 0xB0);
    sectorCountPort.Write(0);
    lbaLowPort.Write(0);
    lbaMidPort.Write(0);
    lbaHiPort.Write(0);
    commandPort.Write(0xEC); // identify command
    
    // Read the Status port (commandPort)(0x1F7) again. If the value read is 0, the drive does not exist
    status = commandPort.Read();
    if(status == 0x00)
        return;
    
    while(((status & 0x80) == 0x80) // if set then device is busy
         && ((status & 0x01) != 0x01)) // if set then there is error
            status = commandPort.Read(); //keep updating status
                
  if(status & 0x01)
    {
        printf("ERROR");
        return;
    }

    for(int i = 0; i < 256; i++) // We are reading 2 bytes from data port so , i will be 256 , so total bytes read are 512
        {
            uint16_t data = dataPort.Read();
            char *text = "  \0";
            text[1] = (data >> 8) & 0xFF;
            text[0] = data & 0xFF;
            printf(text);
        }
    printf("\n");
}

void AdvancedTechnologyAttachment::Read28(common::uint32_t sectorNum, int count)
{
    if(sectorNum > 0x0FFFFFFF)
        return;

    // We are in 28-bit mode and the LBA has 24 bits for the address so there are 4 bits left so we put them in the device port
       devicePort.Write( (master ? 0xE0 : 0xF0) | ((sectorNum & 0x0F000000) >> 24) );
       
       errorPort.Write(0);
       sectorCountPort.Write(1); // We will read or write a single sector
       
        lbaLowPort.Write(  sectorNum & 0x000000FF );
        lbaMidPort.Write( (sectorNum & 0x0000FF00) >> 8);
        lbaHiPort.Write( (sectorNum & 0x00FF0000) >> 16 );
        commandPort.Write(0x20); //0x20 is read command
        
    uint8_t status = commandPort.Read();
    while(((status & 0x80) == 0x80)
       && ((status & 0x01) != 0x01))
        status = commandPort.Read();

    if(status & 0x01)
    {
        printf("ERROR");
        return;
    }


    printf("Reading ATA Drive: ");

    for(int i = 0; i < count; i += 2)
    {
        uint16_t wdata = dataPort.Read();

        char *text = "  \0";
        text[0] = wdata & 0xFF;

        if(i+1 < count)
            text[1] = (wdata >> 8) & 0xFF;
        else
            text[1] = '\0';

        printf(text);
    }    

    for(int i = count + (count%2); i < 512; i += 2)
        dataPort.Read();
}

void AdvancedTechnologyAttachment::Write28(common::uint32_t sectorNum, common::uint8_t* data, common::uint32_t count)
{
    if(sectorNum > 0x0FFFFFFF)
        return;
    
    // Refuse writing more than a sector
    if(count > 512)
        return;
    
    devicePort.Write( (master ? 0xE0 : 0xF0) | ((sectorNum & 0x0F000000) >> 24) );
    errorPort.Write(0);
    sectorCountPort.Write(1);
    lbaLowPort.Write(  sectorNum & 0x000000FF );
    lbaMidPort.Write( (sectorNum & 0x0000FF00) >> 8);
    lbaHiPort.Write( (sectorNum & 0x00FF0000) >> 16 );
    commandPort.Write(0x30);


    printf("Writing to ATA Drive: ");

    for(int i = 0; i < count; i += 2)
    {
        uint16_t wdata = data[i];
        if(i+1 < count)
            wdata |= ((uint16_t)data[i+1]) << 8;  //if the next byte is available we write it also
            
       dataPort.Write(wdata);

        char *text = "  \0";
        text[1] = (wdata >> 8) & 0xFF;
        text[0] = wdata & 0xFF;
        printf(text);
    }

    // if we write bytes less than full size of the sector we write zeroes to the rest of the sector , and if count is an odd number then the first byte must have been written in the wdata loop as zero so we cover this case and write the second byte of the 2 bytes only as zero also
    for(int i = count + (count%2); i < 512; i += 2)
        dataPort.Write(0x0000);
    
}


void AdvancedTechnologyAttachment::Flush()
{
    devicePort.Write( master ? 0xE0 : 0xF0 );
    commandPort.Write(0xE7);  // flush command

uint8_t status = commandPort.Read();
    if(status == 0x00)
        return;

    while(((status & 0x80) == 0x80)
       && ((status & 0x01) != 0x01))
        status = commandPort.Read();

    if(status & 0x01)
    {
        printf("ERROR");
        return;
    }
}

    
    






    
