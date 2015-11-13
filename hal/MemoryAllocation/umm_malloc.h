// ----------------------------------------------------------------------------
// umm_malloc.h - a memory allocator for embedded systems (microcontrollers)
//
// See copyright notice in LICENSE.TXT
// ----------------------------------------------------------------------------

#ifndef UMM_MALLOC_H
#define UMM_MALLOC_H

// ----------------------------------------------------------------------------
#include "umm_malloc_cfg.h"   //-- user-dependent



typedef struct UMM_HEAP_INFO_t {
   unsigned short int totalEntries;
   unsigned short int usedEntries;
   unsigned short int freeEntries; 

   unsigned short int totalBlocks; 
   unsigned short int usedBlocks; 
   unsigned short int freeBlocks; 
}
UMM_HEAP_INFO;

extern UMM_HEAP_INFO heapInfo;

extern char   __umm_heap_start[];
extern char   __umm_heap_end[];
extern size_t __umm_heap_size;

void *umm_info( void *ptr, int force );

void *umm_malloc( size_t size );
void *umm_realloc( void *ptr, size_t size );
void umm_free( void *ptr );

unsigned int OSGetFreeHeapSize( void );
unsigned int OSGetUsedHeapSize( void );


// ----------------------------------------------------------------------------

#endif // UMM_MALLOC_H
