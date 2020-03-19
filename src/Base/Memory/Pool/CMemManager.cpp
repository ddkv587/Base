#include "CBase.hpp"

#include <malloc.h>

namespace BASE
{
    static BOOLEAN      s_bInitialized = FALSE;
    static CMemManager  g_memManager;

    CMemManager* CMemManager::getInstance()
    {
        if ( s_bInitialized )
        {
            return &g_memManager;
        }
        else
        {
            return NULL;
        }
    }

    CMemManager::CMemManager()
        : m_pListener( NULL )
        , m_pMemAllocator( NULL )
        , m_pMemChecker( NULL )
        , m_bCallbacking( FALSE )
    {
        initialize();

        s_bInitialized = TRUE;
    }

    CMemManager::~CMemManager()
    {
        outputState(0);

        s_bInitialized = FALSE;

        if ( m_pMemChecker )
        {
            delete m_pMemChecker;
            m_pMemChecker = NULL;
        }

        if ( m_pMemAllocator )
        {
            delete m_pMemAllocator;
            m_pMemAllocator = NULL;
        }
    }

    void CMemManager::setListener( IMemListener* pListener )
    {
        ::std::unique_lock<SMUTEX> ulock( m_mutexCallback );
        m_pListener = pListener;
    }

    void* CMemManager::malloc( SIZE size, const STRING& strClassName, UINT uiClassID )
    {
        void* pRet = NULL;

        {
            if ( m_pMemChecker ) {
                pRet = m_pMemChecker->malloc( size, uiClassID );
            } else if ( m_pMemAllocator ) {
                pRet = m_pMemAllocator->malloc( size );
            } else {
                pRet = ::malloc( size );
            }
        }

        if ( pRet ) {
            return pRet;
        }

        printf( "[Warning][BASE] Out of Memory: Class <%s> requests %u Bytes, thread_id=%u.\n",
                !strClassName.empty() ? strClassName : L"UnregisteredClass or AtomType", size, ::std::this_thread::get_id() );

        {
            ::std::unique_lock<SMUTEX> ulock( m_mutexCallback );
            if ( m_pListener && !m_bCallbacking ) {
                m_bCallbacking = TRUE;
                m_pListener->onOutOfMemory( size );

                // Try to alloc again.
                {
                    if ( m_pMemChecker ) {
                        pRet = m_pMemChecker->malloc( size, uiClassID );
                    } else if ( m_pMemAllocator ) {
                        pRet = m_pMemAllocator->malloc(size);
                    } else {
                        pRet = ::malloc(size);
                    }
                }

                if ( pRet ) {
                    printf( "[Warning][BASE] Failed to allocate memory (size: %u Bytes).\n", size );

                    m_pListener->onMemoryError();
                }

                m_bCallbacking = FALSE;
            }
        }

        return pRet;
    }

    void CMemManager::free( void* ptr )
    {
        if ( m_pMemChecker ) {
            m_pMemChecker->free( ptr );
        } else if ( m_pMemAllocator ) {
            m_pMemAllocator->free( ptr );
        } else {
            ::free( ptr );
        }
    }

    INT CMemManager::checkPtr(void* p, const STRING& strHint)
    {
        if ( m_pMemChecker ) {
            return m_pMemChecker->checkPtr( p, strHint );
        }

        return -1;
    }

    UINT CMemManager::registClassName( const STRING& strClassName )
    {
        if ( m_pMemChecker ) {
            return m_pMemChecker->registClassName( strClassName );
        }

        return 0;
    }

    BOOLEAN CMemManager::outputState( UINT uiGPUMemorySize )
    {
        if ( m_pMemChecker ) {
            return m_pMemChecker->outputState( uiGPUMemorySize );
        }

        return FALSE;
    }

    BOOLEAN CMemManager::generatePoolConfig( const String& strFileName, UINT uiIncBytes )
    {
        if ( m_pMemChecker ) {
            return m_pMemChecker->generatePoolConfig( strFileName, uiIncBytes );
        }

        return FALSE;
    }

    void CMemManager::registThreadName( UINT tID, const String& strName )
    {
        if ( m_pMemChecker ) {
            m_pMemChecker->registThreadName( tID, strName );
        }
    }

    void CMemManager::initialize()
    {
        CMemoryParser memParser;

        String strPath = CEnvironment::getEnv(ENV_VAR_MEMORY_INI_PATH);
        if (strPath.length() == 0)
        {
            strPath = CPath::getApplicationFolder() + L"Memory.ini";
        }
        if (memParser.parse(strPath) == FALSE)
        {
            return;
        }

        if (memParser.getUseAllocator())
        {
            m_pMemAllocator = new CMemAllocator();

            UINT uiCount = memParser.getPoolConfigSize();
            m_pMemAllocator->initialize(memParser.getMemAlignByte(), uiCount);

            UINT uiMemCheckExtSize = memParser.getMemoryCheck() ? CMemChecker::getBlockExtSize() : 0;

            for (UINT uiIndex = 0; uiIndex < uiCount; ++uiIndex)
            {
                CMemoryParser::tagPoolConfig poolConfig = memParser.getPoolConfigByIndex(uiIndex);

                m_pMemAllocator->createPool(poolConfig.uiUnitSize + uiMemCheckExtSize,
                                            poolConfig.uiInitCount,
                                            poolConfig.uiMaxCount,
                                            poolConfig.uiAppendCount);
            }
        }

        if (memParser.getMemoryCheck())
        {
            m_pMemChecker = new CMemChecker();
            m_pMemChecker->initialize(memParser.getMemroyCompactSizeRange(), m_pMemAllocator);
            m_pMemChecker->setReportFile(memParser.getMemoryReportFile());
        }
    }
}