#ifndef __MYOS__GDT_H
#define __MYOS__GDT_H

#include <common/types.h>

namespace myos
{


    class GlobalDescriptorTable
    {
    
    public:
        
        class SegmentDescriptor
        {
        private:
            myos::common::uint16_t limit_lo;                                      //lo means least significant bits of the segment descriptor and hi means the most significant bits
            myos::common::uint16_t base_lo;
            myos::common::uint8_t base_hi;
            myos::common::uint8_t type;
            myos::common::uint8_t limit_hi;
            myos::common::uint8_t base_vhi;
            
        public:
            SegmentDescriptor(myos::common::uint32_t base, myos::common::uint32_t limit, myos::common::uint8_t type);
            myos::common::uint32_t Base();                                    //This method computes the Base address of the segment.
            myos::common::uint32_t Limit();                                   //This method computes the Limit of the segment.
        }__attribute__((packed));                               // We use the attribute 'packed' to tell GCC not to change any of the alignment in the structure and not to do any optimizations.
                                                                
        
    private:
        SegmentDescriptor nullSegmentSelector;
        SegmentDescriptor unusedSegmentSelector;
        SegmentDescriptor codeSegmentSelector;
        SegmentDescriptor dataSegmentSelector;
        
    public:
        GlobalDescriptorTable();
        ~GlobalDescriptorTable();
        
        myos::common::uint16_t CodeSegmentSelector();
        myos::common::uint16_t DataSegmentSelector();
        //These methods are supposed to return the offset of the segments in the GDT.
    };
}
#endif
