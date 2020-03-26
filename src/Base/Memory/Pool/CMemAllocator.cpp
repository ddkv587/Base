#include <iostream>

#include "CBase.hpp"

namespace Base
{
    #define SIZE_UNIT_NODE_HEARDER                      sizeof( tagUnitNode )
    #define SIZE_POOL_BLOCK_HEARDER                     sizeof( tagPoolBlock )

    #define PTR_UNIT_NODE_HEADER(ptr_unit_data)         PUnitNode((BYTE*)ptr_unit_data - SIZE_UNIT_NODE_HEARDER)
    #define PTR_UNIT_NODE_DATA(ptr_unit_hdr)            (void*)((BYTE*)ptr_unit_hdr + SIZE_UNIT_NODE_HEARDER)

    #define PTR_POOL_BLOCK_DATA(ptr_hdr, data_offset)   (void*)((BYTE*)ptr_hdr + data_offset)

    #define UNIT_NODE_MAGIC                             0x837a46c2
    #define MAKE_UNIT_NODE_MAGIC(ptr_unit_hdr)          (UNIT_NODE_MAGIC ^ (UINT)ptr_unit_hdr)

    #define MEM_ALIGN_BYTE_DEFAULT                      ((SIZE)4U)
    #define MEM_ALIGN_MASK_DEFAULT                      (MEM_ALIGN_BYTE_DEFAULT - (SIZE)1U)

    #define MEM_ALIGN_PAD_SIZE(req_size, align_mask)    ((req_size + (SIZE)align_mask) & ~(SIZE)align_mask)

    #define PINT_SET_VALUE(ptr_int, ivalue) \
        if (ptr_int != NULL) \
        { \
            *ptr_int = ivalue; \
        }

    ////////////////////////////////////////////////////////////////////////////////////////////

    CMemAllocator::CMemAllocator()
        : m_pAryMemPool(NULL)
        , m_uiMaxPoolCount(0)
        , m_uiCurPoolCount(0)
        , m_amHookLock( ATOMIC_VAR_INIT( 0 ) )
    {
        m_uiMemAlignMask = MEM_ALIGN_MASK_DEFAULT;
        m_uiBlockDataOffset = MEM_ALIGN_PAD_SIZE(SIZE_POOL_BLOCK_HEARDER + SIZE_UNIT_NODE_HEARDER, m_uiMemAlignMask) - SIZE_UNIT_NODE_HEARDER;
        m_uiSysChunkHdrSize = MEM_ALIGN_PAD_SIZE(SIZE_UNIT_NODE_HEARDER, m_uiMemAlignMask);

        printf("[Info][BASE] Memory allocator started.\n");
    }

    CMemAllocator::~CMemAllocator()
    {
        freeAllPool();

        printf("[Info][BASE] Memory allocator exited.\n");
    }

    BOOLEAN CMemAllocator::initialize( const JSON& jConfig, UINT uiExtSize )
    {
        auto jPool = jConfig["pool"];

        initialize( jConfig["memory_align_byte"].get<UINT>(), jPool.size() );

        printf("+++++++++++++++++++++\n");
        for ( auto it = jPool.begin(); it != jPool.end(); ++it ) {
            if( !createPool( 
                    (*it)["unit_size"].get<UINT>() + uiExtSize,
                    (*it)["init_count"].get<UINT>(),
                    (*it)["max_count"].get<UINT>(),
                    (*it)["append_count"].get<UINT>() ) ) {
                return FALSE;
            }
        }

        return TRUE;
    }

    void CMemAllocator::initialize(UINT uiMemAlignByte, UINT uiMaxPoolCount)
    {
        if ( uiMemAlignByte > 0 ) {
            m_uiMemAlignMask = ((uiMemAlignByte + 1) & ~1) - 1;
            m_uiSysChunkHdrSize = MEM_ALIGN_PAD_SIZE(SIZE_UNIT_NODE_HEARDER, m_uiMemAlignMask);
            m_uiBlockDataOffset = MEM_ALIGN_PAD_SIZE(SIZE_POOL_BLOCK_HEARDER + SIZE_UNIT_NODE_HEARDER, m_uiMemAlignMask) - SIZE_UNIT_NODE_HEARDER;
        }

        if ( uiMaxPoolCount == 0 ) {
            return;
        }

        m_uiMaxPoolCount = uiMaxPoolCount;
    }

    BOOLEAN CMemAllocator::createPool( SIZE szUnitSize, UINT uiInitCount, UINT uiMaxCount, UINT uiAppendCount )
    {
        if ( m_mapMemPool.size() >= m_uiMaxPoolCount ) {
            return FALSE;
        }

        SIZE szUnitChunkSize = MEM_ALIGN_PAD_SIZE( szUnitSize + SIZE_UNIT_NODE_HEARDER, m_uiMemAlignMask );
        SIZE szUnitAvailSize = szUnitChunkSize - SIZE_UNIT_NODE_HEARDER;
        {
            m_amHookLock.fetch_add( 1, ::std::memory_order_consume );

            auto it = m_mapMemPool.find( szUnitAvailSize );

            if ( it == m_mapMemPool.end() ) {
                // insert
                m_mapMemPool.emplace( ::std::piecewise_construct,
                                        std::forward_as_tuple( szUnitAvailSize ),
                                        std::forward_as_tuple( szUnitChunkSize, szUnitAvailSize, uiInitCount, uiMaxCount, uiAppendCount, 0 ) );
            } else {
                // merge
                auto pool = it->second;
                pool.m_uiMaxCount     = pool.m_uiMaxCount == 0 ? 0 : pool.m_uiMaxCount + uiMaxCount;
                pool.m_uiAppendCount  = MAX( pool.m_uiAppendCount, uiAppendCount );
            }

            m_amHookLock.fetch_sub( 1, ::std::memory_order_consume );
        }

        return TRUE;
    }

    BOOLEAN CMemAllocator::getPoolState( UINT uiIndex, tagMemPoolState& poolState )
    {
        if ( uiIndex >= m_mapMemPool.size() ) {
            return FALSE;
        }

        ::std::lock_guard<SMUTEX> lock( m_syncMutex );
        
        auto it = m_mapMemPool.begin();
        ::std::advance( it, uiIndex );

        PMemPool pMemPool = &( it->second );

        poolState.szUnitAvailSize   = pMemPool->m_szUnitAvailSize;
        poolState.uiMaxCount        = pMemPool->m_uiMaxCount;
        poolState.uiCurrentCount    = pMemPool->m_uiCurrentCount;

        poolState.uiFreeCount = 0;
        if ( pMemPool->m_uiCurrentCount > 0 ) {
            PUnitNode pUnitNode = pMemPool->m_pFreedLink;
            while ( pUnitNode ) {
                ++poolState.uiFreeCount;
                pUnitNode = pUnitNode->pNextUnit;
            }
            poolState.uiFreeCount += (pMemPool->m_pCurBlock->uiUnitCount - pMemPool->m_pCurBlock->uiUsedCursor);
        }

        poolState.szMemoryCost = 0;
        if ( pMemPool->m_uiCurrentCount > 0 ) {
            PPoolBlock pPoolBlock = pMemPool->m_pCurBlock;
            while ( pPoolBlock ) {
                poolState.szMemoryCost += pPoolBlock->uiBlockSize;
                pPoolBlock = pPoolBlock->pNextBlock;
            }
        }

        return TRUE;
    }

    void* CMemAllocator::malloc(SIZE size)
    {
        if ( m_amHookLock.fetch_add( 1, ::std::memory_order_acquire ) ) {
            m_amHookLock.fetch_sub( 1, ::std::memory_order_consume );

            return ::malloc( size );
        }

        auto it = m_mapMemPool.lower_bound( size );

        if ( it != m_mapMemPool.end() ) {
            void* ptr = allocUnit( &( it->second ) );

            printf("[Info][BASE] CMemAllocator malloc from pool, size: %lld, ptr: %p, magic: %lld\n", size, PTR_UNIT_NODE_DATA(pUnitNode), ( (PUnitNode) PTR_UNIT_NODE_HEADER( ptr ) )->szMagic );
            return ptr;
        } else {
            void* pRet = ::malloc(m_uiSysChunkHdrSize + size);    // make sure memory alignment

            if ( pRet )
            {
                PUnitNode pUnitNode = PUnitNode( (BYTE*)pRet + m_uiSysChunkHdrSize - SIZE_UNIT_NODE_HEARDER);

                pUnitNode->pMemPool = NULL;
                pUnitNode->szMagic  = MAKE_UNIT_NODE_MAGIC(pUnitNode);

                printf("[Info][BASE] CMemAllocator malloc, size: %lld, ptr: %p, magic: %lld\n", size, PTR_UNIT_NODE_DATA(pUnitNode), pUnitNode->szMagic );
            

                return PTR_UNIT_NODE_DATA(pUnitNode);
            }
        }
    }

    void CMemAllocator::free(void* p)
    {
        tagUnitNode* pUnitNode = PTR_UNIT_NODE_HEADER( p );

        if ( pUnitNode->szMagic != MAKE_UNIT_NODE_MAGIC( pUnitNode ) ) {
            printf("[Warning][BASE] Free(0x%x): Invalid Pointer, Maybe it had been freed or crashed.\n", p);
            return;
        }

        if ( pUnitNode->pMemPool ) {
            ::std::lock_guard<SMUTEX> lock( m_syncMutex );

            printf("[Info][BASE] CMemAllocator free size to free link: %p, magic: %lld\n", p, pUnitNode->szMagic );

            pUnitNode->pNextUnit                = pUnitNode->pMemPool->m_pFreedLink;
            pUnitNode->pMemPool->m_pFreedLink   = pUnitNode;
        } else {
            printf("[Info][BASE] CMemAllocator free size: %p, magic: %lld\n", p, pUnitNode->szMagic );

            ::free( (BYTE*)p - m_uiSysChunkHdrSize );
        }
    }

    BOOLEAN  CMemAllocator::addPoolBlock( tagMemPool* pMemPool )
    {
        if ( ( pMemPool->m_uiMaxCount > 0 ) && ( pMemPool->m_uiCurrentCount >= pMemPool->m_uiMaxCount ) )
        {
            return FALSE;
        }

        UINT uiAddCount = 0;
        if ( pMemPool->m_pCurBlock == NULL && pMemPool->m_uiInitCount != 0 ) {
            uiAddCount = pMemPool->m_uiInitCount;
        } else {
            uiAddCount = pMemPool->m_uiAppendCount;
        }

        if ( ( pMemPool->m_uiMaxCount > 0 ) && ( ( uiAddCount + pMemPool->m_uiCurrentCount ) > pMemPool->m_uiMaxCount ) ) {
            uiAddCount = pMemPool->m_uiMaxCount - pMemPool->m_uiCurrentCount;
        }

        if ( 0 == uiAddCount ) {
            printf("[Error][BASE] CMemAllocator::addPoolBlock: Pool(index = %u) config has error, please check Memory.ini.\n",
                    (m_pAryMemPool - pMemPool) / sizeof(tagMemPool) + 1);
            return FALSE;
        }

        UINT uiBlockSize =	MEM_ALIGN_PAD_SIZE( SIZE_POOL_BLOCK_HEARDER + SIZE_UNIT_NODE_HEARDER, m_uiMemAlignMask )
                                + pMemPool->m_szUnitChunkSize * uiAddCount - SIZE_UNIT_NODE_HEARDER;
        PPoolBlock pPoolBlock = (PPoolBlock)::malloc( uiBlockSize );
        if ( !pPoolBlock )
        {
            printf("[Warning][BASE] Allocate memory pool block failed (size = %u).\n", uiBlockSize);
            return FALSE;
        }

        pPoolBlock->uiBlockSize     = uiBlockSize;
        pPoolBlock->uiUnitCount     = uiAddCount;
        pPoolBlock->uiUsedCursor    = 0;
        pPoolBlock->pNextBlock      = pMemPool->m_pCurBlock;

        pMemPool->m_pCurBlock       = pPoolBlock;
        pMemPool->m_uiCurrentCount  += uiAddCount;

        return TRUE;
    }

    void CMemAllocator::freeAllPool()
    {
        UINT uiIndex = 0;
        while (uiIndex < m_uiCurPoolCount)
        {
            tagMemPool* pMemPool = m_pAryMemPool + uiIndex;

            PPoolBlock pPoolBlock = pMemPool->m_pCurBlock;
            while (pPoolBlock)
            {
                PPoolBlock pTempBlock = pPoolBlock;
                pPoolBlock = pPoolBlock->pNextBlock;
                ::free(pTempBlock);
            }
            pMemPool->m_pCurBlock = NULL;

            ++uiIndex;
        }

        m_uiCurPoolCount = 0;

        ::free(m_pAryMemPool);
        m_pAryMemPool = NULL;
        m_uiMaxPoolCount = 0;
    }

    void* CMemAllocator::allocUnit( tagMemPool* pMemPool )
    {
        ::std::lock_guard<SMUTEX> lock( m_syncMutex );

        PUnitNode pUnitNode = pMemPool->m_pFreedLink;

        if ( pUnitNode != NULL ) {
            pMemPool->m_pFreedLink  = pUnitNode->pNextUnit;
            pUnitNode->szMagic      = MAKE_UNIT_NODE_MAGIC(pUnitNode);

            printf( "malloc from free link : %lld\n", pUnitNode->szMagic );

            return PTR_UNIT_NODE_DATA(pUnitNode);
        }

        tagPoolBlock* pPoolBlock = pMemPool->m_pCurBlock;

        if ( !pPoolBlock || pPoolBlock->uiUsedCursor >= pPoolBlock->uiUnitCount) {
            if ( addPoolBlock( pMemPool ) ) {
                pPoolBlock = pMemPool->m_pCurBlock;
            } else {
                return NULL;
            }
        }

        pUnitNode = PUnitNode( (BYTE*)PTR_POOL_BLOCK_DATA( pPoolBlock, m_uiBlockDataOffset ) + pMemPool->m_szUnitChunkSize * pPoolBlock->uiUsedCursor );
        pPoolBlock->uiUsedCursor++;

        pUnitNode->szMagic  = MAKE_UNIT_NODE_MAGIC(pUnitNode);
        pUnitNode->pMemPool = pMemPool;

        printf( "malloc from pool: %lld\n", pUnitNode->szMagic );

        return PTR_UNIT_NODE_DATA(pUnitNode);
    }
}
