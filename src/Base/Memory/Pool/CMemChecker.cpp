#include <fstream>
#include "CBase.hpp"

namespace Base
{
////////////////////////////////////////////////////////////////////////

    #define MAGIC_WORD            0xCDDCABBA
    #define MAGIC_FREE            0x32235445
    #define MAGIC_TAIL            ((CHAR) 0xd7)

    static constexpr SIZE_HEARDER       = sizeof( tagBlockHeader );
    static constexpr SIZE_MAGIC_TAIL    = sizeof( CHAR );

    #define PTR_BLOCK_HEADER(ptr_data)                      PBlockHeader( (BYTE*)ptr_data - SIZE_HEARDER )
    #define PTR_BLOCK_DATA(ptr_hdr)                         (void*)( (BYTE*)ptr_hdr + SIZE_HEARDER )
    #define PTR_BLOCK_MAGIC_TAIL(ptr_hdr, data_size)        (char*)( (BYTE*)ptr_hdr + SIZE_HEARDER + data_size )

////////////////////////////////////////////////////////////////////////

    SIZE CMemChecker::getBlockExtSize()
    {
        return SIZE_HEARDER + SIZE_MAGIC_TAIL;
    }

    CMemChecker::CMemChecker()
        : m_pMemAllocator(NULL)
        , m_uiReportID(0)
        , m_pBlockList(NULL)
        , m_bCompactSizeRange(FALSE)
        , m_uiBadPtrAccess(0)
        , m_iHookLock(0)
        , m_mapClassName(TRUE)
        , m_uiThreadCount( 0 )
        , m_amHookLock( ATOMIC_VAR_INIT( 0 ) )
    {
        fprintf( stderr, "[Info][BASE] Memory check started.\n" );

        memset( m_threadSize, 0 , sizeof( tagThreadSize ) * SIZE_INFO_MAX_COUNT );
    }

    CMemChecker::~CMemChecker()
    {
        fprintf( stderr, "[Info][BASE] Memory check exited.\n" );
    }

    void CMemChecker::initialize(BOOLEAN bCompactSizeRange, CMemAllocator* pMemAllocator)
    {
        m_bCompactSizeRange = bCompactSizeRange;
        m_pMemAllocator     = pMemAllocator;

        initSizeRange();
    }

    void* CMemChecker::malloc( SIZE size, UINT uiClassID )
    {
        ::std::lock_guard<SMUTEX> lock( m_syncMutex );

        if ( !m_amHookLock.load( ::std::memory_order_acquire ) ) {
            return hookMalloc(size, uiClassID);
        } else {
            return ::malloc(size);
        }
    }

    void CMemChecker::free( void* ptr )
    {
        if ( !ptr )   return;

        ::std::lock_guard<SMUTEX> lock( m_syncMutex );

        if ( !m_amHookLock.load( ::std::memory_order_acquire ) ) {
            hookFree( ptr );
        } else {
            ::free( ptr );
        }
    }

    INT CMemChecker::checkPtr( void* ptr, const STRING& strHint )
    {
        if ( !ptr ) {
            // fprintf( stderr, "[Warning][BASE] CheckPtr() at <%ls>: The Pointer is NULL!\n", strHint);
            return (INT)bsFreed;
        }

        ::std::lock_guard<SMUTEX> lock( m_syncMutex );

        if ( !m_amHookLock.load( ::std::memory_order_acquire ) ) {
            EBlockStatus status = checkBlock( PTR_BLOCK_HEADER( str ) );

            if ( status == bsOk )   return 0;

            if ( strHint == STRING_NULL ) {
                strHint = "Unknown";
            }

            switch ( status ) {
            case bsFreed:
                fprintf( stderr, "[Warning][BASE] CheckPtr(0x%x) at <%ls>: Invalid Pointer, it had been freed before!\n", str, strHint );
                break;
            case bsHeaderError:
                fprintf( stderr, "[Warning][BASE] CheckPtr(0x%x) at <%ls>: Invalid Pointer, Maybe it had been freed or crashed!\n", str, strHint);
                break;
            case bsTailError:
                fprintf( stderr, "[Warning][BASE] CheckPtr(0x%x) at <%ls>: Pointer is Ok, but block was overflowed!\n", str, strHint);
                break;
            default:
                break;
            }

            return (INT)status;
        }

        return 0;
    }

    UINT CMemChecker::registClassName( const STRING& strClassName )
    {
        if ( strClassName == STRING_NULL ) {
            return 0;
        }

        if ( !m_amHookLock.load( ::std::memory_order_acquire ) ) {
        {
            ::std::lock_guard<SMUTEX> lock( m_syncMutex );

            ++m_amHookLock;

            for ( UINT i = 0; i < m_aryClassName.size(); ++i ) {
                if ( m_aryClassName[i] == strClassName ) {
                    return i;
                }
            }

            m_aryClassName.push_back( strClassName );

            --m_amHookLock;

            return m_aryClassName.size() - 1;
        }

        return 0;
    }

    void CMemChecker::registThreadName( UINT uiThreadID, const STRING& strThreadName )
    {
        if ( strThreadName.empty() )  return;

        if ( !m_amHookLock.load( ::std::memory_order_acquire ) ) {
            ::std::lock_guard<SMUTEX> lock( m_syncMutex );
            m_mapThreadName[ uiThreadID ] = strThreadName;
        }
    }

    void CMemChecker::setReportFile(const String& strReportFile)
    {
        ::std::lock_guard<SMUTEX> lock( m_syncMutex );

        m_strReportFile = strReportFile;
    }

    BOOLEAN CMemChecker::outputState(UINT uiGPUMemorySize)
    {
        if ( !m_strReportFile.empty() ) {
            ::std::fstream fs;

            fs.open( m_strReportFile, ::std::ios::out );

            if ( fs.is_open() ) {
                return outputStateToLogger( fs );
            }
        }

        return FALSE;
    }

    UINT CMemChecker::getThreadCount()
    {
        return m_uiThreadCount;
    }

    UINT CMemChecker::getThreadID( UINT uiIndex )
    {
        if ( uiIndex < m_uiThreadCount ) return m_threadSize[uiIndex].tID;

        return 0;
    }

    SIZE CMemChecker::getThreadSize( UINT uiIndex )
    {
        if ( uiIndex < m_uiThreadCount ) return m_threadSize[uiIndex].tSize;

        return 0;
    }

    BOOLEAN CMemChecker::generatePoolConfig(const String& strFileName, UINT uiIncBytes)
    {
        // generate new mem pool config

        return TRUE;
    }

    void* CMemChecker::hookMalloc( SIZE size, UINT uiClassID )
    {
        UINT uiSize = size + getBlockExtSize();
        PBlockHeader pHeader = NULL;

        {
            if ( m_pMemAllocator ) {
                pHeader = (PBlockHeader)( m_pMemAllocator->malloc( uiSize ) );
            } else {
                pHeader = (PBlockHeader)::malloc(uiSize);
            }
        }

        if ( !pHeader )     return NULL;

        {
            ::std::lock_guard<SMUTEX> lock( m_syncMutex );

            pHeader->szSize         = size;
            pHeader->uiClassID      = uiClassID;
            pHeader->tThreadID      = ::std::this_thread::get_id();
            *( PTR_BLOCK_MAGIC_TAIL( pHeader, size ) ) = MAGIC_TAIL;

            linkBlock( pHeader );

            logAllocSize(size);

            UINT ui = 0;
            for ( ; ui < m_uiThreadCount; ++ui ) {
                if ( m_threadSize[ui].tID == pHeader->uiThreadID )
                {
                    break;
                }
            }
            if ( ui == m_uiThreadCount ) ++m_uiThreadCount;

            m_threadSize[ui].tID    = pHeader->uiThreadID;
            m_threadSize[ui].tSize  += pHeader->uiSize;
        }

        return PTR_BLOCK_DATA(pHeader);
    }

    void CMemChecker::hookFree(void* p)
    {
        PBlockHeader pHeader = PTR_BLOCK_HEADER(p);
        EBlockStatus status = checkBlock(pHeader);

        if (status == bsFreed)
        {
            m_uiBadPtrAccess++;
            if (pHeader->uiClassID != 0)
            {
                fprintf( stderr, "[Warning][BASE] Free(0x%X)<%ls>: Invalid Pointer. Block had been freed before!\n", p, (const STRING&)(*m_mapClassName.getValueByKey(pHeader->uiClassID)));
            }
            else
            {
                fprintf( stderr, "[Warning][BASE] Free(0x%X): Invalid Pointer. Block had been freed before!\n", p);
            }
            return;
        }

        if (status == bsHeaderError) {
            m_uiBadPtrAccess++;
            fprintf( stderr, "[Warning][BASE] Free(0x%X): Invalid Pointer. Maybe Block had been freed or crashed!\n", p);
            return;
        }

        if (status == bsTailError)
        {
            m_uiBadPtrAccess++;
            if (pHeader->uiClassID != 0)
            {
                fprintf( stderr, "[Warning][BASE] Free(0x%X)<%ls>: Block was overflow!\n", p, (const STRING&)(*m_mapClassName.getValueByKey(pHeader->uiClassID)));
            }
            else
            {
                fprintf( stderr, "[Warning][BASE] Free(0x%X): Block was overflow!\n", p);
            }
        }

        {
            CAutoLocker autoLocker(this, TRUE);

            unlinkBlock(pHeader);

            logFreedSize(pHeader->uiSize);

            UINT ui = 0;
            for ( ; ui < m_uiThreadCount; ++ui )
            {
                if ( m_threadSize[ui].tID == pHeader->uiThreadID )
                {
                    break;
                }
            }
            if ( ui == m_uiThreadCount ) ++m_uiThreadCount;

            m_threadSize[ui].tID    = pHeader->uiThreadID;
            m_threadSize[ui].tSize  -= pHeader->uiSize;
        }

        {
            if (m_pMemAllocator)
            {
                m_pMemAllocator->free(pHeader);
            }
            else
            {
                ::free(pHeader);
            }
        }
    }

    CMemChecker::EBlockStatus CMemChecker::checkBlock(CMemChecker::PBlockHeader pHeader)
    {
        EBlockStatus status;

        switch (pHeader->uiMagic1 ^ ((UINT)pHeader->pPrev + (UINT)pHeader->pNext))
        {
        case MAGIC_FREE:
            status = bsFreed;
            break;

        case MAGIC_WORD:
            if ((pHeader->uiMagic2 ^ (UINT)pHeader) != MAGIC_WORD)
            {
                status = bsHeaderError;
            }
            else if (*PTR_BLOCK_MAGIC_TAIL(pHeader, pHeader->uiSize) != MAGIC_TAIL)
            {
                status = bsTailError;
            }
            else
            {
                status = bsOk;
            }
            break;

        default:
            status = bsHeaderError;
            break;
        }

        return status;
    }

    CMemChecker::ELinkStatus CMemChecker::checkAllBlock(UINT &uiErrorBlockCount)
    {
        PBlockHeader pBlock = m_pBlockList;

        uiErrorBlockCount = 0;

        while (pBlock != NULL)
        {
            EBlockStatus status = checkBlock(pBlock);
            switch (status)
            {
            case bsHeaderError:
                uiErrorBlockCount++;
                if (pBlock->pNext && checkBlock(pBlock->pNext) == bsHeaderError)
                {
                    // If neighboring block was bad.
                    // We regard this situation as the Block-Link was crashed!
                    // Unable to continue check.
                    return lsLinkCrashed;
                }
                break;

            case bsTailError:
                uiErrorBlockCount++;
                break;

            case bsFreed:
                // It's impossible to here.
                break;

            default:
                break;
            }

            pBlock = pBlock->pNext;
        }

        if (uiErrorBlockCount >= 0)
        {
            return lsOk;
        }
        else
        {
            return lsHasBlockError;
        }
    }

    void CMemChecker::linkBlock(PBlockHeader pHeader)
    {
        pHeader->pPrev = NULL;
        pHeader->pNext = m_pBlockList;
        m_pBlockList = pHeader;

        if (pHeader->pNext != NULL)
        {
            pHeader->pNext->pPrev = pHeader;
            pHeader->pNext->uiMagic1 = MAGIC_WORD ^ ((UINT)pHeader + (UINT)(pHeader->pNext->pNext));
        }

        pHeader->uiMagic1 = MAGIC_WORD ^ (UINT)(pHeader->pNext);
        pHeader->uiMagic2 = MAGIC_WORD ^ (UINT)(pHeader);
    }

    void CMemChecker::unlinkBlock(PBlockHeader pHeader)
    {
        if (pHeader->pNext != NULL)
        {
            pHeader->pNext->pPrev = pHeader->pPrev;
            pHeader->pNext->uiMagic1 = MAGIC_WORD ^ ((UINT)(pHeader->pNext->pPrev) + (UINT)(pHeader->pNext->pNext));
        }

        if (pHeader->pPrev != NULL)
        {
            pHeader->pPrev->pNext = pHeader->pNext;
            pHeader->pPrev->uiMagic1 = MAGIC_WORD ^ ((UINT)(pHeader->pPrev->pPrev) + (UINT)(pHeader->pPrev->pNext));
        }
        else
        {
            m_pBlockList = pHeader->pNext;
        }

        pHeader->pPrev = NULL;
        pHeader->pNext = NULL;

        pHeader->uiMagic1 = MAGIC_FREE;
        pHeader->uiMagic2 = MAGIC_FREE;
    }

    BOOLEAN CMemChecker::outputStateToLogger(CLogger* pLogger, UINT uiGPUMemorySize)
    {
        ELinkStatus linkStatus = lsOk;

        CTime time = CTime::getCurrentTime();
        String strTime = String::format(L"--- Report_%.4d at %04d-%02d-%02d %02d:%02d:%02d.%03d ---\n", ++m_uiReportID,
                                        time.getYear(), time.getMonth(), time.getDay(),
                                        time.getHour(), time.getMinute(), time.getSecond(), time.getMilliseconds());
        pLogger->writeLine(strTime);

        // --- Block Errors ---
        if (m_blockStatAll.uiCurrentCount != 0)
        {
            UINT uiErrorBlockCount = 0;
            linkStatus = checkAllBlock(uiErrorBlockCount);

            if (linkStatus == lsLinkCrashed)
            {
                pLogger->writeLine(L"[Warning] Heap was crashed! Please check this issue ASAP.\n");
            }
            else if (linkStatus == lsHasBlockError)
            {
                pLogger->writeLine(String::format(L"[Warning] %u block error was detected!\n", uiErrorBlockCount));
            }
        }

        if (m_uiBadPtrAccess != 0)
        {
            pLogger->writeLine(String::format(L"[Warning] Tried to Free invalid pointer %u times! Please check the Log of Terminal.\n", m_uiBadPtrAccess));
        }

        if (pLogger->hasError())
        {
            return FALSE;
        }

        if (pLogger->lineCount() > 1)
        {
            pLogger->writeLine(L"\n");
        }

        // --- Size range ---
        pLogger->writeLine(L"\"SizeRange\",\"AllocTimes\",\"CurrentCount\",\"CurrentSize\",\"PeakCount\",\"PeakSize\", \"AverageSize\"\n");

        for (INT i = 0; i < SIZE_INFO_MAX_COUNT; ++i)
        {
            if (m_blockStatSize[i].llAllocTimes == 0)
            {
                continue;
            }

            FLOAT fAvgSize = 0;
            if (m_blockStatSize[i].uiCurrentCount != 0)
            {
                fAvgSize = FLOAT(m_blockStatSize[i].uiCurrentSize) / m_blockStatSize[i].uiCurrentCount;
            }

            pLogger->writeLine(String::format(    L"\"[%u, %u]\",\"%lld\",\"%u\",\"%u\",\"%u\",\"%u\",\"%.1f\"\n",
                                                m_blockStatSize[i].uiBeginSize,
                                                m_blockStatSize[i].uiEndSize,
                                                m_blockStatSize[i].llAllocTimes,
                                                m_blockStatSize[i].uiCurrentCount,
                                                m_blockStatSize[i].uiCurrentSize,
                                                m_blockStatSize[i].uiPeakCount,
                                                m_blockStatSize[i].uiPeakSize,
                                                fAvgSize));

            if (pLogger->hasError())
            {
                return FALSE;
            }
        }

        FLOAT fAvgSizeAll = 0;
        if (m_blockStatAll.uiCurrentCount != 0)
        {
            fAvgSizeAll = FLOAT(m_blockStatAll.uiCurrentSize) / m_blockStatAll.uiCurrentCount;
        }

        pLogger->writeLine(String::format(    L"\"<All>\",\"%lld\",\"%u\",\"%u\",\"%u\",\"%u\",\"%.1f\"\n",
                                            m_blockStatAll.llAllocTimes,
                                            m_blockStatAll.uiCurrentCount,
                                            m_blockStatAll.uiCurrentSize,
                                            m_blockStatAll.uiPeakCount,
                                            m_blockStatAll.uiPeakSize,
                                            fAvgSizeAll));

        // Pool state
        if (m_pMemAllocator)
        {
            pLogger->writeLine(L"\n");

            UINT uiPoolMemoryCost = 0;
            UINT uiCheckExtSize = getBlockExtSize();

            pLogger->writeLine(L"\"PoolIndex\",\"MemoryCost\",\"UnitSize\",\"MaxCount\",\"CurrentCount\",\"FreeCount\",\"FreePercent\"\n");

            for (UINT uiIndex = 0; uiIndex < m_pMemAllocator->getPoolCount(); ++uiIndex)
            {
                CMemAllocator::tagMemPoolState poolState;

                m_pMemAllocator->getPoolState(uiIndex, poolState);

                poolState.uiUnitAvailSize -= uiCheckExtSize;

                if (poolState.uiCurrentCount != 0)
                {
                    poolState.uiMemoryCost -= uiCheckExtSize * poolState.uiCurrentCount;
                    FLOAT fFreePercent = FLOAT(poolState.uiFreeCount * 100.0) / poolState.uiCurrentCount;

                    pLogger->writeLine(String::format(    L"\"%u\",\"%u\",\"%u\",\"%u\",\"%u\",\"%u\",\"%.1f%%\"\n",
                                                        uiIndex + 1,
                                                        poolState.uiMemoryCost,
                                                        poolState.uiUnitAvailSize,
                                                        poolState.uiMaxCount,
                                                        poolState.uiCurrentCount,
                                                        poolState.uiFreeCount,
                                                        fFreePercent));
                }
                else
                {
                    pLogger->writeLine(String::format(    L"\"%u\",\"0\",\"%u\",\"%u\",\"0\",\" \",\" \"\n",
                        uiIndex + 1,
                        poolState.uiUnitAvailSize,
                        poolState.uiMaxCount));
                }

                if (pLogger->hasError())
                {
                    return FALSE;
                }

                uiPoolMemoryCost += poolState.uiMemoryCost;
            }

            pLogger->writeLine(String::format(L"\"<All>\",\"%u\"\n", uiPoolMemoryCost));
        }

        // Exit if all block were freed.
        if (m_blockStatAll.uiCurrentCount == 0)
        {
            return (!pLogger->hasError());
        }

        pLogger->writeLine(L"\n");

        // --- Class & Thread info ---
        MapThreadStat mapThreadStat(TRUE);
        buildClassStat(mapThreadStat);

        if (linkStatus != lsOk)
        {
            pLogger->writeLine(L"NOTE: The following info is not exact, because Heap or Block was crashed.\n");
        }

        pLogger->writeLine(L"\"\",\"ClassName\",\"InstanceCount\",\"TotalSize\"\n");
        // --- Class Info ---
        for ( UINT i = 0; i < mapThreadStat.size(); ++i )
        {
            //pLogger->writeLine(String::format(L"\"<GPU Used(Estimate)>\",\"---\",\"%u\"\n", uiGPUMemorySize));
            MapPair< UINT, MapClassStat > pair = mapThreadStat.getKeyValueByIndex(i);

            UINT            uiThreadID        = *( pair.pKey );
            MapClassStat*    mapClassStat    = pair.pData;

            String* pStrThreadName = m_mapThreadName.getValueByKey( uiThreadID );
            if ( pStrThreadName )
            {
                pLogger->writeLine( String::format( L"\"Thread-%d ( %ls )\"\n" , uiThreadID, pStrThreadName->buffer() ) );
            }
            else
            {
                pLogger->writeLine( String::format( L"\"Thread-%d ( %ls )\"\n" , uiThreadID, CThread::getThreadNameByID( uiThreadID ).buffer() ) );
            }

            for ( UINT uiIndex = 0; uiIndex < mapClassStat->size(); ++uiIndex )
            {
                const String& strClassName = mapClassStat->getKeyByIndex(uiIndex);
                const tagClassStat* pClassStat = mapClassStat->getValueByIndex(uiIndex);

                if (strClassName == STRING_NULL)
                {
                    pLogger->writeLine(String::format(    L"\"\",\"<UnregisteredClass or AtomType>\",\"%u\",\"%u\"\n",
                                                        pClassStat->uiInstanceCount,
                                                        pClassStat->uiTotalSize));
                }
                else
                {
                    pLogger->writeLine(String::format(    L"\"\",\"%ls\",\"%u\",\"%u\"\n",
                                                        (const STRING&)(strClassName),
                                                        pClassStat->uiInstanceCount,
                                                        pClassStat->uiTotalSize));
                }

                if (pLogger->hasError())
                {
                    return FALSE;
                }
            }
            pLogger->writeLine(L"\n");
        }

        pLogger->writeLine(L"\n");

        return (!pLogger->hasError());
    }

    void CMemChecker::initSizeRange()
    {
        if (m_bCompactSizeRange == FALSE)
        {
            //    [1,4] [5,8] [9,16] [17,32] [33,64] ... [0x80000000, 0xFFFFFFFF]

            // uiIndex = 0
            m_blockStatSize[0].uiBeginSize = 1;
            m_blockStatSize[0].uiEndSize = 4;

            // uiIndex = 1~29
            UINT uiIndex = 1;
            while (uiIndex <= 29)
            {
                m_blockStatSize[uiIndex].uiBeginSize = (1 << (uiIndex + 1)) + 1;
                m_blockStatSize[uiIndex].uiEndSize = 2 << (uiIndex + 1);
                ++uiIndex;
            }

            // uiIndex = 30
            m_blockStatSize[uiIndex].uiBeginSize = 0x80000001;
            m_blockStatSize[uiIndex].uiEndSize = 0xFFFFFFFF;
        }
        else
        {
            //    [1,4] [5,8] [9,12] [13,16] [17,20] ...[509, 512]  ==> [513, 1024] ... [0x80000000, 0xFFFFFFFF]

            // uiIndex = 0 ~ 127
            UINT uiIndex = 0;
            for ( ; uiIndex <= 127; ++uiIndex)
            {
                m_blockStatSize[uiIndex].uiBeginSize = uiIndex * 4 + 1;
                m_blockStatSize[uiIndex].uiEndSize = (uiIndex + 1) * 4;
            }

            // uiIndex = 128 ~ 149
            SIZE sizeTemp = 512;
            while (sizeTemp <= 0x40000000)
            {
                m_blockStatSize[uiIndex].uiBeginSize = sizeTemp + 1;
                sizeTemp <<= 1;
                m_blockStatSize[uiIndex].uiEndSize = sizeTemp;
                ++uiIndex;
            }

            // uiIndex = 150
            m_blockStatSize[uiIndex].uiBeginSize = 0x80000001;
            m_blockStatSize[uiIndex].uiEndSize = 0xFFFFFFFF;
        }
    }

    INT CMemChecker::calcRangeIndex(SIZE size)
    {
        if (m_bCompactSizeRange == FALSE)
        {
            if (size <= 4)
            {
                return 0;
            }

            UINT uiIndex = 1;
            SIZE sizeTemp = (size - 1) >> 3;
            while (sizeTemp > 0)
            {
                sizeTemp >>= 1;
                ++uiIndex;
            }

            return uiIndex;
        }
        else
        {
            if (size <= 4)
            {
                return 0;
            }

            if (size <= 512)
            {
                return (size - 1) / 4;
            }
            else
            {
                UINT uiIndex = 128;

                SIZE sizeTemp = (size - 1) >> 10;
                while (sizeTemp > 0)
                {
                    sizeTemp >>= 1;
                    ++uiIndex;
                }

                return uiIndex;
            }
        }
    }

    void CMemChecker::logAllocSize(SIZE size)
    {
        m_blockStatAll.llAllocTimes++;
        m_blockStatAll.uiCurrentCount++;
        m_blockStatAll.uiCurrentSize += size;
        if (m_blockStatAll.uiPeakCount < m_blockStatAll.uiCurrentCount)
        {
            m_blockStatAll.uiPeakCount = m_blockStatAll.uiCurrentCount;
        }
        if (m_blockStatAll.uiPeakSize < m_blockStatAll.uiCurrentSize)
        {
            m_blockStatAll.uiPeakSize = m_blockStatAll.uiCurrentSize;
        }

        INT iIndex = calcRangeIndex(size);

        m_blockStatSize[iIndex].llAllocTimes++;
        m_blockStatSize[iIndex].uiCurrentCount++;
        m_blockStatSize[iIndex].uiCurrentSize += size;
        if (m_blockStatSize[iIndex].uiPeakCount < m_blockStatSize[iIndex].uiCurrentCount)
        {
            m_blockStatSize[iIndex].uiPeakCount = m_blockStatSize[iIndex].uiCurrentCount;
        }
        if (m_blockStatSize[iIndex].uiPeakSize < m_blockStatSize[iIndex].uiCurrentSize)
        {
            m_blockStatSize[iIndex].uiPeakSize = m_blockStatSize[iIndex].uiCurrentSize;
        }
    }

    void CMemChecker::logFreedSize(SIZE size)
    {
        m_blockStatAll.uiCurrentCount--;
        m_blockStatAll.uiCurrentSize -= size;

        INT iIndex = calcRangeIndex(size);

        m_blockStatSize[iIndex].uiCurrentCount--;
        m_blockStatSize[iIndex].uiCurrentSize-= size;
    }

    void CMemChecker::buildClassStat(MapThreadStat &mapThreadStat)
    {
        PBlockHeader pBlock = m_pBlockList;

        while (pBlock != NULL)
        {
            if (checkBlock(pBlock) == bsHeaderError)
            {
                // If neighboring block was bad.
                // We regard this situation as the Block-Link was crashed!
                pBlock = pBlock->pNext;
                if (checkBlock(pBlock) == bsHeaderError)
                {
                    return;
                }
                continue;
            }

            // Collect by thread id first
            MapClassStat* mapClassStat = mapThreadStat.getValueByKey( pBlock->uiThreadID );
            if ( !mapClassStat )
            {
                mapClassStat = new MapClassStat( TRUE );
                mapThreadStat.addItem( pBlock->uiThreadID, mapClassStat );
            }

            // Collect as Class
            const String* pStrClassName = NULL;
            if (pBlock->uiClassID != 0)
            {
                pStrClassName = m_mapClassName.getValueByKey(pBlock->uiClassID);
            }
            if (pStrClassName == NULL)
            {
                pStrClassName = &STRING_NULL;
            }

            tagClassStat* pClassStat = mapClassStat->getValueByKey(*pStrClassName);

            if (pClassStat != NULL)
            {
                pClassStat->uiInstanceCount++;
                pClassStat->uiTotalSize += pBlock->uiSize;
            }
            else
            {
                pClassStat = new tagClassStat();

                pClassStat->uiInstanceCount = 1;
                pClassStat->uiTotalSize = pBlock->uiSize;

                mapClassStat->addItem(String(*pStrClassName), pClassStat);
            }

            pBlock = pBlock->pNext;
        }
    }
}