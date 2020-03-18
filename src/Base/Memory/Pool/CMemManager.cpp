#include "CBase.hpp"

#include <malloc.h>

namespace BASE
{
    static BOOLEAN		s_bInitialized = FALSE;
    static CMemManager	g_memManager;

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
        : m_pListener(NULL)
        , m_pMemAllocator(NULL)
        , m_pMemChecker(NULL)
        , m_bCallbacking(FALSE)
    {
        initialize();

        s_bInitialized = TRUE;
    }

    CMemManager::~CMemManager()
    {
        outputState(0);

        s_bInitialized = FALSE;

        if (m_pMemChecker != NULL)
        {
            delete m_pMemChecker;
            m_pMemChecker = NULL;
        }

        if (m_pMemAllocator != NULL)
        {
            delete m_pMemAllocator;
            m_pMemAllocator = NULL;
        }
    }

    void CMemManager::setListener(IMemListener* pListener)
    {
        CAutoSync autoSync(m_syncCallback);
        m_pListener = pListener;
    }

    void* CMemManager::malloc(size_t size, const CHAR* pcClassName, UINT uiClassID)
    {
        void* pRet = NULL;
        {
            if (m_pMemChecker != NULL)
            {
                pRet = m_pMemChecker->malloc(size, uiClassID);
            }
            else if (m_pMemAllocator != NULL)
            {
                pRet = m_pMemAllocator->malloc(size);
            }
            else
            {
                pRet = ::malloc(size);
            }
        }

        if (pRet != NULL)
        {
            return pRet;
        }

        wprintf(L"[Warning][BASE] Out of Memory: Class <%ls> requests %u Bytes, thread_id=%u.\n",
                pcClassName != NULL ? pcClassName : L"UnregisteredClass or AtomType", size, CThread::getCurrentThreadId());

        if (m_pListener != NULL && m_bCallbacking == FALSE)
        {
            CAutoSync autoSync(m_syncCallback);

            if (m_pListener != NULL)
            {
                m_bCallbacking = TRUE;

                m_pListener->onOutOfMemory(size);

                // Try to alloc again.
                {
                    if (m_pMemChecker != NULL)
                    {
                        pRet = m_pMemChecker->malloc(size, uiClassID);
                    }
                    else if (m_pMemAllocator != NULL)
                    {
                        pRet = m_pMemAllocator->malloc(size);
                    }
                    else
                    {
                        pRet = ::malloc(size);
                    }
                }

                if (pRet == NULL)
                {
                    wprintf(L"[Warning][BASE] Failed to allocate memory (size: %u Bytes).\n", size);

                    m_pListener->onMemoryError();
                }

                m_bCallbacking = FALSE;
            }
        }

        return pRet;
    }

    void CMemManager::free(void* p)
    {
        if (m_pMemChecker != NULL)
        {
            m_pMemChecker->free(p);
        }
        else if (m_pMemAllocator != NULL)
        {
            m_pMemAllocator->free(p);
        }
        else
        {
            ::free(p);
        }
    }

    INT CMemManager::checkPtr(void* p, const CHAR* pcHint)
    {
        if (m_pMemChecker != NULL)
        {
            return m_pMemChecker->checkPtr(p, pcHint);
        }

        return -1;
    }

    UINT CMemManager::registClassName(const CHAR* pcClassName)
    {
        if (m_pMemChecker != NULL)
        {
            return m_pMemChecker->registClassName(pcClassName);
        }

        return 0;
    }

    BOOLEAN CMemManager::outputState(UINT uiGPUMemorySize)
    {
        if (m_pMemChecker != NULL)
        {
            return m_pMemChecker->outputState(uiGPUMemorySize);
        }

        return FALSE;
    }

    BOOLEAN CMemManager::generatePoolConfig(const String& strFileName, UINT uiIncBytes)
    {
        if (m_pMemChecker != NULL)
        {
            return m_pMemChecker->generatePoolConfig(strFileName, uiIncBytes);
        }

        return FALSE;
    }

    void CMemManager::registThreadName( UINT tID, const String& strName )
    {
        if ( m_pMemChecker )
        {
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