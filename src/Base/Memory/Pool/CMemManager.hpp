/*
 * @Author: ddkV587 
 * @Date: 2020-03-19 15:09:05 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-23 17:40:13
 */

#ifndef DEF__BASE_CMEMMANAGER_HPP__
#define DEF__BASE_CMEMMANAGER_HPP__

namespace Base
{
    class CMemManager
    {
    public:// const define

    private:// const define

    public:// embed class or struct or enum

    private:// embed class or struct or enum

    public:// method
        static CMemManager*     getInstance();

        // inner method
        CMemManager();
        virtual ~CMemManager();

        void*                   malloc( SIZE size, const STRING& strClassName = STRING_NULL, UINT uiClassID = 0 );
        void                    free(void* p);
        INT                     checkPtr(void* p, const STRING& strHint = STRING_NULL );

        UINT                    registClassName( const STRING& strClassName );
        BOOLEAN                 outputState( UINT uiGPUMemorySize = 0 );

        BOOLEAN                 hasMemChecker()                             { return ( NULL != m_pMemChecker ); }

        // =============== get total alloc state =================
       // UINT        getCurrentAllocCount()                      { return m_pMemChecker ? m_pMemChecker->getCurrentAllocCount() : 0; }
        //SIZE        getCurrentAllocSize()                       { return m_pMemChecker ? m_pMemChecker->getCurrentAllocSize() : 0; }

        // =============== get alloc state by thread =================
       // UINT        getThreadCount()                            { return m_pMemChecker ? m_pMemChecker->getThreadCount() : 0; }
       // UINT        getThreadID( UINT uiIndex )                 { return m_pMemChecker ? m_pMemChecker->getThreadID( uiIndex ) : 0; }
        //SIZE        getThreadSize( UINT uiIndex )               { return m_pMemChecker ? m_pMemChecker->getThreadSize( uiIndex ) : 0; }

        BOOLEAN                 generatePoolConfig( const STRING& strFileName, UINT uiIncBytes );

        void                    registThreadName( UINT tID, const STRING& strName );

    protected:// method
        CMemManager( CMemManager& ) = delete;
        CMemManager( CMemManager&& ) = delete;
        CMemManager& operator=( CMemManager&& ) = delete;

    private:// method
        void                    initialize();

    protected:// property

    private:// property
        CMemAllocator*              m_pMemAllocator;
        CMemChecker*                m_pMemChecker;
       
        SMUTEX                      m_mutexCallback;
        BOOLEAN                     m_bCallbacking;
    };
}

#endif