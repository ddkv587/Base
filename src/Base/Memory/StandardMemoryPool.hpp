#ifndef __STANDARDMEMORYPOOLHPP__
#define __STANDARDMEMORYPOOLHPP__

namespace NET
{
    class StandardMemoryPool final : public MemoryPool
    {
    public:
        StandardMemoryPool( UINT64 size, BOOLEAN boundCheck );
        ~StandardMemoryPool();

        inline virtual void*    allocate( UINT64 size );
        inline virtual void     free( void* ptrc );
        inline virtual bool     integrityCheck() const;
        inline virtual void     dumpToFile(const STRING& fileName, const UINT64 itemsPerLine) const;

    private:
    
    private: 
        BYTE*		            m_poolMemory;
    };
}
#endif