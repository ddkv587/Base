#ifndef __MEMORYPOOLHPP__
#define __MEMORYPOOLHPP__

namespace NET
{
#ifdef _DEBUG_
#define TRASH_POOLS 1
#else
#define TRASH_POOLS 0
#endif
    class MemoryPool
    {
    public:
        static const BYTE       s_trashOnCreation = 0xCC;
        static const BYTE       s_trashOnAllocSignature = 0xAB;
        static const BYTE       s_trashOnFreeSignature  = 0xFE;
        static const BYTE       s_boundsCheckSize = 16;
        static const BYTE       s_startBound[s_boundsCheckSize];
        static const BYTE       s_endBound[s_boundsCheckSize];

        inline virtual void*    allocate( UINT64 size ) = 0;
        inline virtual void     free( void* ptrc ) = 0;
        inline virtual bool     integrityCheck() const = 0;
        inline virtual void     dumpToFile(const STRING& fileName, const UINT64 itemsPerLine) const = 0;

    protected:
        MemoryPool()
            : m_totalPoolSize( 0 )
            , m_freePoolSize( 0 )
            , m_trashOnCreation( TRASH_POOLS )
            , m_trashOnAlloc( TRASH_POOLS )
            , m_trashOnFree( TRASH_POOLS )
            , m_boundsCheck( 0 )
        {
            ;
        }
        virtual ~MemoryPool(){};

        UINT64          m_totalPoolSize;
        UINT64          m_freePoolSize;

        UINT            m_trashOnCreation : 1;
        UINT            m_trashOnAlloc : 1;
        UINT            m_trashOnFree : 1;
        UINT            m_boundsCheck : 1;
    };
}

#endif