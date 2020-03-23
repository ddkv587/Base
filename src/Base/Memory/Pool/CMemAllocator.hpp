/*
 * @Author: ddkV587 
 * @Date: 2020-03-19 15:09:25 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-23 18:23:37
 */

#ifndef __BASE_CMEMALLOCATOR_HPP__
#define __BASE_CMEMALLOCATOR_HPP__

namespace BASE
{
    class CMemAllocator
    {
    public:// const define

    private:// const define

    public:// embed class or struct or enum

        struct tagMemPoolState
        {
            UINT    uiUnitAvailSize;
            UINT    uiMaxCount;
            UINT    uiCurrentCount;
            UINT    uiFreeCount;
            UINT    uiMemoryCost;
            //UINT  uiWasteCost;
        };

    private:// embed class or struct or enum
        struct tagMemPool;

        struct tagUnitNode
        {
            tagMemPool*         pMemPool;
            union
            {
                tagUnitNode*    pNextUnit;
                UINT            uiMagic;
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
            UINT            m_uiUnitChunkSize;
            UINT            m_uiUnitAvailSize;

            UINT            m_uiInitCount;
            UINT            m_uiMaxCount; // "MaxCount == 0" means no limited.
            UINT            m_uiAppendCount;
            UINT            m_uiCurrentCount;

            tagPoolBlock*   m_pFirstBlock;
            tagUnitNode*    m_pFreedLink;
        };
        typedef tagMemPool* PMemPool;

    public:// method
        // inner method
        CMemAllocator();
        virtual ~CMemAllocator();

        BOOLEAN         initialize( const JSON& jConfig, UINT uiExtSize = 0 );
        void            initialize( UINT uiMemAlignByte, UINT uiMaxPoolCount);

        BOOLEAN         createPool(UINT uiUnitSize, UINT uiInitCount, UINT uiMaxCount, UINT uiAppendCount);
        UINT            getPoolCount() { return m_uiCurPoolCount; };
        BOOLEAN         getPoolState(UINT uiIndex, tagMemPoolState& poolState);

        void*           malloc(SIZE size);
        void            free(void* p);

    protected:
        // Inner method

    private: // method
        BOOLEAN         addPoolBlock(tagMemPool* pMemPool);
        tagMemPool*     findFitPool(SIZE size, INT* piIndex = NULL);
        void            freeAllPool();

        void*           allocUnit(tagMemPool* pMemPool);

    public: // property

    private:// property
        tagMemPool*     m_pAryMemPool;
        UINT            m_uiMaxPoolCount;
        UINT            m_uiCurPoolCount;

        UINT            m_uiMemAlignMask;

        UINT            m_uiBlockDataOffset;
        UINT            m_uiSysChunkHdrSize;

        CSyncObject     m_syncObject;
    };
}

#endif