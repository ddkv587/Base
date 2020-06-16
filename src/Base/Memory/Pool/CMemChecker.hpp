/*
 * @Author: ddkV587 
 * @Date: 2020-03-19 15:09:18 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-23 18:49:56
 */

#ifndef DEF__BASE_CMEMCHECKER_HPP__
#define DEF__BASE_CMEMCHECKER_HPP__

#include <execinfo.h>

namespace Base
{
    class CMemChecker
    {
    public:// const define

    private:// const define

        static const INT SIZE_INFO_MAX_COUNT = 151;

    public:// embed class or struct or enum

    private:// embed class or struct or enum

        enum EBlockStatus
        {
            bsOk            = 0,
            bsFreed         = 1,
            bsHeaderError   = 2,
            bsTailError     = 3,
        };

        enum ELinkStatus
        {
            lsOk            = 0,
            lsHasBlockError = 1,
            lsLinkCrashed   = 2
        };

        struct tagBlockStat
        {
            SIZE                szBoundUp;
            SIZE                szBoundDown;
            
            SIZE                szAllocTimes;
            SIZE                szCurrentCount;
            SIZE                szCurrentSize;
            SIZE                szPeakCount;
            SIZE                szPeakSize;
        };

        
        struct tagBlockHeader
        {
            SIZE                szMagiHeader;
            UINT                uiClassID;          // Key of Class name, zero means unknown.
            STHREAD_ID          tThreadID;          // thread id for check
            SIZE                szSize;             // Exact size requested by user.
            STRING              strBackTrace;       // backtrace for each block

            tagBlockHeader*     pNext;
            tagBlockHeader*     pPrev;
            SIZE                szMagicTail;
        };
        typedef tagBlockHeader* PBlockHeader;

        struct tagThreadSize
        {
            STHREAD_ID          tID;
            SIZE                tSize;
        };

        struct tagClassStat
        {
            SIZE                szInstanceCount;
            SIZE                szTotalSize;

            PBlockHeader        pUsedBlock;
        };

        typedef SMAP< STRING, tagClassStat >                _MapClassStat;
        typedef SMAP< STHREAD_ID, _MapClassStat >           _MapThreadStat;

    public:// method
        static SIZE         getBlockExtSize();

        CMemChecker() ;
        virtual ~CMemChecker();

        void                initialize(BOOLEAN bCompactSizeRange, CMemAllocator* pMemAllocator = NULL);

        void*               malloc( SIZE size, UINT uiClassID = 0 );
        void                free( void* ptr );
        INT                 checkPtr( void* ptr, const STRING& pcHint = NULL );

        UINT                registClassName( const STRING& pcClassName );
        void                registThreadName( UINT uiThreadID, const STRING& strThreadName);

        void                setReportFile (const STRING& );
        BOOLEAN             outputState( ::std::fstream& );

        // =============== get total alloc state =================
        SIZE                getCurrentAllocSize()       { return m_blockStatAll.uiCurrentSize; }
        UINT                getCurrentAllocCount()      { return m_blockStatAll.uiCurrentCount; }

        // =============== get alloc state by thread =================
        UINT                getThreadCount();
        STHREAD_ID          getThreadID( UINT uiIndex );
        SIZE                getThreadSize( UINT uiIndex );

        BOOLEAN             generatePoolConfig( const STRING& strFileName = STRING_NULL );

    protected:

    private: // method
        void                lock( BOOLEAN bCtrlHook );
        void                unlock( BOOLEAN bCtrlHook );

        void*               hookMalloc( SIZE size, UINT uiClassID );
        void                hookFree( void* ptr );

        EBlockStatus        checkBlock( PBlockHeader pHeader );
        ELinkStatus         checkAllBlock( UINT &uiErrorBlockCount );
        void                linkBlock( PBlockHeader pHeader );
        void                unlinkBlock( PBlockHeader pHeader );

        BOOLEAN             outputStateToLogger( CLogger* pLogger );

        void                initSizeRange();
        INT                 calcRangeIndex( SIZE size );
        void                logAllocSize( SIZE size );
        void                logFreedSize( SIZE size );

        void                buildClassStat( MapThreadStat &mapThreadStat );

    public: // property

    private:// property
        CMemAllocator*          m_pMemAllocator;

        String                  m_strReportFile;
        BOOLEAN                 m_bBackTraceEnable;

        PBlockHeader            m_pBlockList;

        BOOLEAN                 m_bCompactSizeRange;
        tagBlockStat            m_blockStatAll;
        tagBlockStat            m_blockStatSize[SIZE_INFO_MAX_COUNT];
        UINT                    m_uiBadPtrAccess;

        SMUTEX                  m_syncMutex;
        ::std::atomic<INT>      m_amHookLock;

        UINT                    m_uiThreadCount;
        tagThreadSize           m_threadSize[SIZE_INFO_MAX_COUNT];

        SVECTOR<STRING>         m_aryClassName;
        SMAP<UINT, STRING>      m_mapThreadName;
    };
}

#endif