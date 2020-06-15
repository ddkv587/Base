/*
 * @Author: ddkV587 
 * @Date: 2020-03-18 17:15:22 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-18 17:45:31
 */

#ifndef DEF__BASE_MEMORY_HPP__
#define DEF__BASE_MEMORY_HPP__

namespace Base
{
    class Memory
    {
    public:// const define

    private:// const define

    public:// embed class or struct or enum

    private:// embed class or struct or enum

    public:// method
        static void*    traceMalloc( SIZE size, const STRING& strClassName = STRING_NULL, UINT uiClassID = 0 );
        static void     traceFree( void* ptr );
        static INT      checkPtr( void* ptr, const STRING& strHint = STRING_NULL );
        static UINT     registClassName( const STRING& strClassName );

    protected:// method
        Memory( Memory& ) = delete;
        Memory( Memory&& ) = delete;
        Memory& operator=( Memory&& ) = delete;

    private:// method

    protected:// property

    private:// property
    };
}

#endif