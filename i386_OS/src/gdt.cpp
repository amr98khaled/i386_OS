#include <gdt.h>
using namespace myos;
using namespace myos::common;

GlobalDescriptorTable::GlobalDescriptorTable()          
:nullSegmentSelector(0,0,0),                           //member initializer list
unusedSegmentSelector(0,0,0),
codeSegmentSelector(0,64*1024*1024,0x9A),           /* 0x9A[10011010b] is put in bits 8:15 in the segment descriptor structure :
                                                    1st flags [bits 12:15 ]: ( present )1 ( privilege )00 ( descriptor type )1 -> 1001b
                                                    type flags [bits 8:11] : ( code )1 ( conforming )0 ( readable )1 ( accessed )0 -> 1010b */

dataSegmentSelector(0,64*1024*1024,0x92)            /* 0x92[10010010b] is put in bits 8:15 in the segment descriptor structure :
                                                    1st flags [bits 12:15 ]: ( present )1 ( privilege )00 ( descriptor type )1 -> 1001b
                                                    type flags [bits 8:11] : ( code )0 ( expand down )0 ( writable )1 ( accessed )0 -> 0010b */
{
    //i is the GDT descriptor and it consists of two elements : 1- 2 bytes for the size of GDT . 2- 4 bytes for the Base address of the GDT .
    uint32_t i[2];                                                                                                                                          
    i[1]=(uint32_t)this;
    //Shift by 2 bytes because lgdt instruction will take only 6 bytes for GDT descriptor [Highest 2 bytes of GDT size and 4 bytes of GDT base address.
    i[0]=sizeof(GlobalDescriptorTable)<<16;
    asm volatile("lgdt (%0)": :"p" (((uint8_t *) i)+2));
}

GlobalDescriptorTable::~GlobalDescriptorTable()
{
}

uint16_t GlobalDescriptorTable::DataSegmentSelector()
{
    return (uint8_t*)&dataSegmentSelector - (uint8_t*)this;
}

uint16_t GlobalDescriptorTable::CodeSegmentSelector()
{
    return (uint8_t*)&codeSegmentSelector - (uint8_t*)this;
}

GlobalDescriptorTable::SegmentDescriptor::SegmentDescriptor(uint32_t base, uint32_t limit, uint8_t type)
{
    uint8_t* target=(uint8_t*)this;             //target is a pointer to the segment entry in GDT.
    
    if(limit <= 65536)
    {   //16-bit address space
        target[6]= 0x40;            //target[6] manipulates bits 23:16 in the segment entry. Granularity bit is set so limit is multiplied by 4k !
    }
    else
    {
        // 32-bit address space
        // Now we have to squeeze the (32-bit) limit into 2.5 regiters (20-bit).
        // This is done by discarding the 12 least significant bits, but this
        // is only legal, if they are all ==1, so they are implicitly still there

        // so if the last bits aren't all 1, we have to set them to 1, but this
        // would increase the limit (cannot do that, because we might go beyond
        // the physical limit or get overlap with other segments) so we have to
        // compensate this by decreasing a higher bit (and might have up to
        // 4095 wasted bytes behind the used memory)

        if((limit & 0xFFF) != 0xFFF)
        {
            limit = (limit>>12)-1;
        }
        else
        {
            limit = limit>>12;
        }
        
        target[6]=0xC0;
    }
    
    //Encode the Limit
    
    target[0]= limit & 0xFF;
    target[1]= (limit>>8) & 0xFF;
    target[6]|= (limit>>16) & 0xF;
    
    //Encode the Base
    
    target[2]= base & 0xFF;
    target[3]= (base>>8) & 0xFF;
    target[4]= (base>>16) & 0xFF;
    target[7]= (base>>24) & 0xFF;
    
    //Type
    
    target[5]= type;
}

uint32_t GlobalDescriptorTable::SegmentDescriptor::Base()
{
    uint8_t* target= (uint8_t*)this;
    
    uint32_t result= target[7];
    result= (result<<8) + target[4];
    result= (result<<8) + target[3];
    result= (result<<8) + target[2];
    return result;
}

uint32_t GlobalDescriptorTable::SegmentDescriptor::Limit()
{
    uint8_t* target= (uint8_t*)this;
    
    uint32_t result= target[6] & 0xF;
    result = (result<<8) + target[1];
    result = (result<<8) + target[0];
    
    if((target[6] & 0xC0)==0xC0)
    {
        result = (result<<12) | 0xFFF;
    }
    
    return result;
}
