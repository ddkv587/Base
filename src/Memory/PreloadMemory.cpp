#include "CMemoryManager.h"

extern "C"
{
    void* malloc( size_t size )
    {
        return MemoryTrace::CMemoryManager::getInstance()->TraceMalloc( size );
    }

    void free( void* ptr )
    {
        MemoryTrace::CMemoryManager::getInstance()->TraceFree( ptr );
    }

    void* calloc( size_t n, size_t len )
    {
        return MemoryTrace::CMemoryManager::getInstance()->TraceCalloc( n, len );
    }

    void* realloc(void *ptr, size_t size)
    {
        return MemoryTrace::CMemoryManager::getInstance()->TraceRealloc( ptr, size );
    }

    void* memalign(size_t blocksize, size_t bytes) 
    {
        return MemoryTrace::CMemoryManager::getInstance()->TraceMemalign( blocksize, bytes );
    }

    void* valloc(size_t size) 
    {
        return MemoryTrace::CMemoryManager::getInstance()->TraceValloc( size );
    }
}