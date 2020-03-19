#include "CBase.hpp"

#include <cstdlib>

namespace Base
{
    void* Memory::malloc( SIZE size, const STRING& strClassName, UINT uiClassID )
    {
        CMemManager* pMemManager = CMemManager::getInstance();

        if ( pMemManager ) {
            return pMemManager->malloc( size, strClassName, uiClassID );
        }

        return ::malloc( size );
    }

    void Memory::free( void* ptr )
    {
        if ( !ptr )
        {
            return;
        }

        CMemManager* pMemManager = CMemManager::getInstance();

        if ( pMemManager ) {
            return pMemManager->free( ptr );
        }

        return ::free( ptr );
    }

    INT Memory::checkPtr( void* ptr, const STRING& strHint )
    {
        CMemManager* pMemManager = CMemManager::getInstance();

        if ( pMemManager ) {
            return pMemManager->checkPtr( ptr );
        }

        return -1;
    }

    UINT Memory::registClassName( const STRING& strClassName )
    {
        CMemManager* pMemManager = CMemManager::getInstance();

        if ( pMemManager ) {
            return pMemManager->registClassName( strClassName );
        }

        return 0;
    }
}
