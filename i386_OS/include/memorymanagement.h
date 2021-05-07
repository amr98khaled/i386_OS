#ifndef __MYOS__MEMORYMANAGEMENT_H
#define __MYOS__MEMORYMANAGEMENT_H

#include <common/types.h>


namespace myos
{

    //MemoryChunk is the header that describes each piece of memory allocated
    struct MemoryChunk
    {
        MemoryChunk *next;
        MemoryChunk *prev;
        bool allocated;
        common::size_t size;
    };


    class MemoryManager
    {

    protected:
        MemoryChunk* first;
    public:

        static MemoryManager *activeMemoryManager;

        MemoryManager(common::size_t start, common::size_t size);           //start is the start address of the first MemoryChunk
        ~MemoryManager();

        void* malloc(common::size_t size);
        void free(void* ptr);  
    };
}


void* operator new(unsigned size);
void* operator new[](unsigned size);

// placement new
void* operator new(unsigned size, void* ptr);
void* operator new[](unsigned size, void* ptr);

void operator delete(void* ptr);
void operator delete[](void* ptr);


#endif 
