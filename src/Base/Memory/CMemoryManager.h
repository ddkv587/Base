#ifndef __CMEMORYMANAGERH__
#define __CMEMORYMANAGERH__

#include <stddef.h>
#include <mutex>

namespace MemoryTrace
{
    class CMemoryManager
    {
    public:
        struct tagUnitNode
        {
            size_t          sign;
            size_t          offset;
            size_t          size;
            void*           pData;
            tagUnitNode*    pPrev;
            tagUnitNode*    pNext;

            bool operator==(const tagUnitNode& pDst)
            {
                return ( ( pDst.sign == sign ) && ( pDst.offset == offset ) && 
                         ( pDst.size == size ) && ( pDst.pData == pData ) &&
                         ( pDst.pPrev == pPrev ) && ( pDst.pNext == pNext ) );
            }
        };

        struct tagUnitManager
        {
            size_t          totalSize;
            size_t          availSize;
            size_t          unitCount;
            tagUnitNode     headUnit;

            tagUnitNode*    pCurrent;
        };

    private:
        typedef void*       (*FUNC_MALLOC)(size_t);
        typedef void*       (*FUNC_CALLOC)(size_t, size_t);
        typedef void*       (*FUNC_REALLOC)(void *, size_t);
        typedef void*       (*FUNC_MEMALIGN)(size_t, size_t);
        typedef void*       (*FUNC_VALLOC)(size_t);
        typedef void        (*FUNC_FREE)(void* );

    public:
        static CMemoryManager*  getInstance();
        static void             delInstance();

        inline bool             isInitialized()             { return m_bInitialized; }

        bool                    TraceInit();
        void*                   TraceMalloc( size_t size );
        void*                   TraceCalloc( size_t nmemb, size_t size );
        void*                   TraceRealloc( void* ptr, size_t size );
        void*                   TraceMemalign( size_t blocksize, size_t size );
        void*                   TraceValloc( size_t size );
        void                    TraceFree( void* ptr );
        
        virtual void            check( bool autoDelete = true );

        CMemoryManager();
        virtual ~CMemoryManager();
        CMemoryManager( CMemoryManager& ) = delete;
        CMemoryManager( const CMemoryManager& ) = delete;

    protected:

    private: 
        void                    appendUnit(tagUnitNode*);
        bool                    deleteUnit(tagUnitNode*);
        
        void*                   _impMalloc( size_t size, bool bRecursive = true );
        void*                   _impCalloc( size_t nmemb, size_t size, bool bRecursive = true );
        void*                   _impRealloc( void* ptr, size_t size, bool bRecursive = true );
        void*                   _impMemalign( size_t blocksize, size_t size, bool bRecursive = true );
        void*                   _impValloc( size_t size, bool bRecursive = true );
        void                    _impFree( void* ptr, bool bRecursive = true );

    private:
        static CMemoryManager*      s_pInstance;

        bool                        m_bInitialized;
        FUNC_MALLOC                 m_pRealMalloc;
        FUNC_CALLOC                 m_pRealCalloc;
        FUNC_REALLOC                m_pRealRealloc;
        FUNC_MEMALIGN               m_pRealMemalign;
        FUNC_VALLOC                 m_pRealValloc;
        FUNC_FREE                   m_pRealFree;
    };
}
#endif