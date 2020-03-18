/*
 * @Author: ddkV587 
 * @Date: 2020-03-18 17:15:22 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-18 17:45:31
 */

#ifndef __BASE_MEMORY_H_INCLUDED__
#define __BASE_MEMORY_H_INCLUDED__

#include <new>

namespace Base
{
    class Memory
    {
    public:// const define

    private:// const define

    public:// embed class or struct or enum

    private:// embed class or struct or enum

    public:// method
        static void*    malloc( SIZE size, const CHAR* pcClassName = NULL, UINT uiClassID = 0 );
        static void     free( void* p );
        static INT      checkPtr( void* p, const CHAR* pcHint = NULL );
        static UINT     registClassName( const CHAR* pcClassName );

    protected:// method
        Memory(const Memory&) {}
        Memory& operator =(const Memory&){return *this;}

    private:// method

    protected:// property

    private:// property
    };
}

#endif