/*
 * @Author: ddkV587 
 * @Date: 2020-03-19 15:09:25 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-26 16:59:32
 */

#ifndef __BASE_CMEMALLOCATOR_HPP__
#define __BASE_CMEMALLOCATOR_HPP__

namespace Base
{
    class CMemAllocator
    {
    public:// const define

    private:// const define

    public:// embed class or struct or enum
        struct tagMemPoolState
        {
            SIZE    szUnitAvailSize;
            UINT    uiMaxCount;
            UINT    uiCurrentCount;
            UINT    uiFreeCount;

            SIZE    szMemoryCost;
            //UINT  uiWasteCost;
        };

    private:// embed class or struct or enum
        struct tagMemPool;

        struct tagUnitNode
        {
            tagMemPool*         pMemPool;
            union
            {
                tagUnitNode*    pNextUnit;          // for free list
                SIZE            szMagic;            // for check
            };
        };
        typedef tagUnitNode* PUnitNode;

        struct tagPoolBlock
        {
            UINT            uiBlockSize;
            UINT            uiUnitCount;
            UINT            uiUsedCursor;

            tagPoolBlock*   pNextBlock;
        };
        typedef tagPoolBlock* PPoolBlock;

        struct tagMemPool
        {
            SIZE            m_szUnitChunkSize;
            SIZE            m_szUnitAvailSize;

            UINT            m_uiInitCount;
            UINT            m_uiMaxCount;       // "MaxCount == 0" means no limited.
            UINT            m_uiAppendCount;
            UINT            m_uiCurrentCount;

            BOOLEAN         m_bAllocated;
            tagPoolBlock*   m_pCurBlock;
            tagUnitNode*    m_pFreedLink;

            tagMemPool( SIZE szChunkSize = 0, SIZE szAvailSize = 0, UINT uiInitCount = 0, UINT uiMaxCount = 0, UINT uiAppendCount = 0, UINT uiCurrentCount = 0 )
                : m_szUnitChunkSize( szChunkSize )
                , m_szUnitAvailSize( szAvailSize )
                , m_uiInitCount( uiInitCount )
                , m_uiMaxCount( uiMaxCount )
                , m_uiAppendCount( uiAppendCount )
                , m_uiCurrentCount( uiCurrentCount )
            {
                m_bAllocated    = FALSE;

                m_pCurBlock     = NULL;
                m_pFreedLink    = NULL;
            }

            void mergeMemPool( tagMemPool& src );
            void reset();
        };
        typedef tagMemPool* PMemPool;

    public:// method
        // inner method
        CMemAllocator();
        virtual ~CMemAllocator();

        BOOLEAN             initialize( const JSON& jConfig, UINT uiExtSize = 0 );
        void                initialize( UINT uiMemAlignByte, UINT uiMaxPoolCount );

        BOOLEAN             createPool( SIZE szUnitSize, UINT uiInitCount, UINT uiMaxCount, UINT uiAppendCount );
        UINT                getPoolCount()                                              { return m_uiCurPoolCount; };
        BOOLEAN             getPoolState( UINT uiIndex, tagMemPoolState& poolState );

        void*               malloc( SIZE size );
        void                free( void* p );

    protected:
        // Inner method

    private: // method
        BOOLEAN             addPoolBlock( tagMemPool* pMemPool );
        tagMemPool*         findFitPool( SIZE size, INT* pIndex = NULL );
        void                freeAllPool();

        void*               allocUnit( tagMemPool* pMemPool );

    public: // property

    private:// property
        tagMemPool*                     m_pAryMemPool;

        UINT                            m_uiMaxPoolCount;
        UINT                            m_uiCurPoolCount;

        UINT                            m_uiMemAlignMask;

        UINT                            m_uiBlockDataOffset;
        UINT                            m_uiSysChunkHdrSize;

        SMUTEX                          m_syncMutex;
        ::std::atomic<INT>              m_amHookLock;

        SMAP< SIZE, tagMemPool >        m_mapMemPool;
    };
}

#endif