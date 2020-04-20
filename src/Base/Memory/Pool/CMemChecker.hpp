/*
 * @Author: ddkV587 
 * @Date: 2020-03-19 15:09:18 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-23 18:49:56
 */

#ifndef DEF__BASE_CMEMCHECKER_HPP__
#define DEF__BASE_CMEMCHECKER_HPP__

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

        struct tagBlockHeader
        {
            UINT                uiMagic1;
            UINT                uiSize;         // Exact size requested by user.
            UINT                uiClassID;      // Key of Class name, zero means unknown.
            UINT                uiThreadID;     // thread id for check
            tagBlockHeader*     pNext;
            tagBlockHeader*     pPrev;
            UINT                uiMagic2;
        };
        typedef tagBlockHeader* PBlockHeader;

        struct tagBlockStat
        {
            SIZE                uiBeginSize;
            SIZE                uiEndSize;
            LLONG               llAllocTimes;
            UINT                uiCurrentCount;
            SIZE                uiCurrentSize;
            UINT                uiPeakCount;
            SIZE                uiPeakSize;

            tagBlockStat()
                : uiBeginSize(0)
                , uiEndSize(0)
                , llAllocTimes(0)
                , uiCurrentCount(0)
                , uiCurrentSize(0)
                , uiPeakCount(0)
                , uiPeakSize(0)
            {
                ;
            }
        };

        struct tagThreadSize
        {
            UINT  tID;
            SIZE  tSize;
        };

        struct tagClassStat
        {
            UINT   uiInstanceCount;
            SIZE   uiTotalSize;

            tagClassStat()
                : uiInstanceCount(0)
                , uiTotalSize(0)
            {
                ;
            }
        };
        typedef SMAP< STRING, tagClassStat >        _MapClassStat;
        typedef SMAP< UINT, MapClassStat >          _MapThreadStat;

    public:// method
        static UINT         getBlockExtSize();

        CMemChecker() ;
        virtual ~CMemChecker();

        void                initialize(BOOLEAN bCompactSizeRange, CMemAllocator* pMemAllocator = NULL);

        void*               malloc( SIZE size, UINT uiClassID = 0 );
        void                free( void* p );
        INT                 checkPtr( void* p, const STRING& pcHint = NULL );

        UINT                registClassName( const STRING& pcClassName );
        void                registThreadName( UINT uiThreadID, const String& strThreadName);

        void                setReportFile (const String& strReportFile );
        BOOLEAN             outputState( UINT uiGPUMemorySize = 0 );

        // =============== get total alloc state =================
        SIZE                getCurrentAllocSize()      { return m_blockStatAll.uiCurrentSize; }
        UINT                getCurrentAllocCount()      { return m_blockStatAll.uiCurrentCount; }

        // =============== get alloc state by thread =================
        UINT                getThreadCount();
        UINT                getThreadID( UINT uiIndex );
        SIZE                getThreadSize( UINT uiIndex );

        BOOLEAN             generatePoolConfig( const String& strFileName, UINT uiIncBytes );

    protected:

    private: // method
        void                lock( BOOLEAN bCtrlHook );
        void                unlock( BOOLEAN bCtrlHook );

        void*               hookMalloc( SIZE size, UINT uiClassID );
        void                hookFree( void* p );

        EBlockStatus        checkBlock( PBlockHeader pHeader );
        ELinkStatus         checkAllBlock( UINT &uiErrorBlockCount );
        void                linkBlock( PBlockHeader pHeader );
        void                unlinkBlock( PBlockHeader pHeader );

       // BOOLEAN     outputStateToLogger( CLogger* pLogger, UINT uiGPUMemorySize );

        void                initSizeRange();
        INT                 calcRangeIndex( SIZE size );
        void                logAllocSize( SIZE size );
        void                logFreedSize( SIZE size );

        void                buildClassStat( MapThreadStat &mapThreadStat );

    public: // property

    private:// property
        CMemAllocator*          m_pMemAllocator;

        String                  m_strReportFile;
        UINT                    m_uiReportID;

        PBlockHeader            m_pBlockList;

        BOOLEAN                 m_bCompactSizeRange;
        tagBlockStat            m_blockStatAll;
        tagBlockStat            m_blockStatSize[SIZE_INFO_MAX_COUNT];
        UINT                    m_uiBadPtrAccess;

        CSyncObject             m_syncObject;
        INT                     m_iHookLock;

        UINT                    m_uiThreadCount;
        tagThreadSize           m_threadSize[SIZE_INFO_MAX_COUNT];

        SMAP<UINT, STRING >     m_mapClassName;
        SMAP<UINT, STRING >     m_mapThreadName;
    };
}

#endif