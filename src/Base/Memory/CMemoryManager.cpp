#include <assert.h>
#include <dlfcn.h>
#include <cstdio>
#include <mutex>
#include <math.h>
#include <pthread.h>
#include "CMemoryManager.h"

namespace MemoryTrace
{
    const static int DEF_SIZE_UNIT_NODE = sizeof(CMemoryManager::tagUnitNode);

    #define PTR_UNIT_NODE_HEADER(ptr_unit_data)			        (tagUnitNode*)( (char*)ptr_unit_data - DEF_SIZE_UNIT_NODE )
	#define PTR_UNIT_NODE_DATA(ptr_unit_hdr)			        (void*)( (char*)ptr_unit_hdr + DEF_SIZE_UNIT_NODE )
    #define PTR_OFFSET_NODE_HEADER(ptr_unit_start, offset)		(tagUnitNode*)( (char*)ptr_unit_start + offset )

    #define UNIT_NODE_MAGIC								        0xFEEF9FF9CDDC9889
	#define MAKE_UNIT_NODE_MAGIC(ptr_unit_hdr)			        ( UNIT_NODE_MAGIC ^ (size_t)ptr_unit_hdr )

    static int s_no_hook = 0;
    static CMemoryManager                   s_Instance;
    static CMemoryManager::tagUnitManager   s_unitManager 
    { 
        .totalSize          = 0,
        .availSize          = 0,
        .unitCount          = 0,
        .headUnit           = {
            .sign   = 0,
            .offset = 0,
            .size   = 0,
            .pData  = NULL,
            .pPrev  = NULL,
            .pNext  = NULL,
        },
        .pCurrent           = &(s_unitManager.headUnit),
    };

    pthread_mutex_t s_mutexInit     = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t s_mutexMemory   = PTHREAD_MUTEX_INITIALIZER;

    CMemoryManager* CMemoryManager::getInstance()
    {
        if ( !s_Instance.isInitialized() ) { assert( s_Instance.TraceInit() ); }
        return &s_Instance;
    }

    void CMemoryManager::delInstance()
    {
        ;
    }

    bool CMemoryManager::TraceInit()
    {
        pthread_mutex_lock( &s_mutexInit );

        if ( m_bInitialized ) return true;

        m_pRealMalloc         = (FUNC_MALLOC)dlsym(RTLD_NEXT, "malloc");
        m_pRealCalloc         = (FUNC_CALLOC)dlsym(RTLD_NEXT, "calloc");
        m_pRealRealloc        = (FUNC_REALLOC)dlsym(RTLD_NEXT, "realloc");
        m_pRealFree           = (FUNC_FREE)dlsym(RTLD_NEXT, "free");
        m_pRealMemalign       = (FUNC_MEMALIGN)dlsym(RTLD_NEXT, "memalign");
        m_pRealValloc         = (FUNC_VALLOC)dlsym(RTLD_NEXT, "valloc");

        if ( NULL == m_pRealMalloc || NULL == m_pRealCalloc || NULL == m_pRealRealloc || 
              NULL == m_pRealFree || NULL == m_pRealMemalign || NULL == m_pRealValloc )
        {
            pthread_mutex_unlock( &s_mutexInit );
            return false;
        }

        m_bInitialized = true;
        pthread_mutex_unlock( &s_mutexInit );

        return true;
    }
    
    void* CMemoryManager::TraceMalloc( size_t size )
    {
        assert( NULL != m_pRealMalloc );

        void* p = _impMalloc( size, __sync_fetch_and_add( &s_no_hook, 1 ) );
        __sync_fetch_and_sub( &s_no_hook, 1 );
        return p;
    }

    void* CMemoryManager::TraceCalloc( size_t nmemb, size_t size )
    {
        assert( NULL != m_pRealCalloc );
        
        void* p = _impCalloc( nmemb, size, __sync_fetch_and_add( &s_no_hook, 1 ) );
        __sync_fetch_and_sub( &s_no_hook, 1 );
        return p;
    }

    void* CMemoryManager::TraceRealloc( void *ptr, size_t size )
    {
        assert( NULL != m_pRealRealloc );

        void* p = _impRealloc( ptr, size, __sync_fetch_and_add( &s_no_hook, 1 ) );
        __sync_fetch_and_sub( &s_no_hook, 1 );
        return p;
    }

    void* CMemoryManager::TraceMemalign( size_t blocksize, size_t bytes )
    {
        assert( NULL != m_pRealMemalign );
 
        void* p = _impMemalign( blocksize, bytes, __sync_fetch_and_add( &s_no_hook, 1 ) );
        __sync_fetch_and_sub( &s_no_hook, 1 );
        return p;
    }
    
    void* CMemoryManager::TraceValloc( size_t size )
    {
        assert( NULL != m_pRealValloc );

        void* p = _impValloc( size, __sync_fetch_and_add( &s_no_hook, 1 ) );
        __sync_fetch_and_sub( &s_no_hook, 1 );
        return p;
    }

    void CMemoryManager::TraceFree( void *ptr )
    {
        assert( NULL != m_pRealFree );
 
        _impFree( ptr, __sync_fetch_and_add( &s_no_hook, 1 ) );
        __sync_fetch_and_sub( &s_no_hook, 1 );
    }

    void CMemoryManager::check( bool autoDelete )
    {
        tagUnitNode* pNode = s_unitManager.headUnit.pNext;

        tagUnitNode* pCur;
        while ( NULL != pNode ) 
        {
            fprintf( stderr, "unfree addr: %p, size: %ld\n", pNode, pNode->size - DEF_SIZE_UNIT_NODE );
            pCur    = pNode;
            pNode   = pNode->pNext;

            if ( autoDelete ) _impFree( pCur );
        }
    }

    CMemoryManager::CMemoryManager()
        : m_bInitialized(false)
        , m_pRealMalloc(NULL)
        , m_pRealCalloc(NULL)
        , m_pRealRealloc(NULL)
        , m_pRealMemalign(NULL)
        , m_pRealValloc(NULL)
        , m_pRealFree(NULL)
    {
        ;
    }

    CMemoryManager::~CMemoryManager()
    {
        check(false);
    }

    void CMemoryManager::appendUnit( tagUnitNode* pNode )
    {
        pthread_mutex_lock( &s_mutexMemory );
        
        if ( NULL == pNode ) { pthread_mutex_unlock( &s_mutexMemory ); return; }
        
        pNode->pPrev = s_unitManager.pCurrent;
        pNode->pNext = NULL;

        assert( NULL != s_unitManager.pCurrent );
        s_unitManager.pCurrent->pNext = pNode;
        s_unitManager.pCurrent = pNode;

        s_unitManager.totalSize += pNode->size;
        s_unitManager.availSize += pNode->size - DEF_SIZE_UNIT_NODE;
        s_unitManager.unitCount += 1;

        pthread_mutex_unlock( &s_mutexMemory );
    }

    bool CMemoryManager::deleteUnit( tagUnitNode* pNode )
    {
        pthread_mutex_lock( &s_mutexMemory );
        
        if ( NULL == pNode ) { pthread_mutex_unlock( &s_mutexMemory ); return true; }

        tagUnitNode* pCheckNode = PTR_OFFSET_NODE_HEADER( &s_unitManager, pNode->offset );

        // hook all type of memory request, this must be true
        assert( MAKE_UNIT_NODE_MAGIC( pNode ) == pNode->sign && (*pCheckNode == *pNode ) );
        assert( NULL != s_unitManager.pCurrent );
        if ( s_unitManager.pCurrent == pCheckNode )
            s_unitManager.pCurrent = pCheckNode->pPrev;

        assert ( NULL != pCheckNode->pPrev );
        pCheckNode->pPrev->pNext = pCheckNode->pNext;

        if ( NULL != pCheckNode->pNext )
            pCheckNode->pNext->pPrev = pCheckNode->pPrev;

        pthread_mutex_unlock( &s_mutexMemory );
        return true;
    }

    void* CMemoryManager::_impMalloc( size_t size, bool bRecursive )
    {
        tagUnitNode* pNode = (tagUnitNode*)m_pRealMalloc( size + DEF_SIZE_UNIT_NODE );

        if ( NULL == pNode ) return NULL;
        pNode->sign     = MAKE_UNIT_NODE_MAGIC( pNode );
        pNode->offset   = (size_t)pNode - (size_t)(&s_unitManager);
        pNode->size     = size + DEF_SIZE_UNIT_NODE;
        pNode->pData    = PTR_UNIT_NODE_DATA( pNode );
        pNode->pPrev    = NULL;
        pNode->pNext    = NULL;
        appendUnit( pNode );

#ifdef _DEBUG
        if ( !bRecursive )
            fprintf(stderr, "===malloc: %p, size: %ld, real: %ld\n", pNode, pNode->size, pNode->size - DEF_SIZE_UNIT_NODE);
#endif
        return PTR_UNIT_NODE_DATA( pNode );
    }

    void* CMemoryManager::_impCalloc( size_t nmemb, size_t size, bool bRecursive )
    {
        nmemb = ceil( (double)( size * nmemb + DEF_SIZE_UNIT_NODE ) / (double)size );
        tagUnitNode* pNode = (tagUnitNode*)m_pRealCalloc( nmemb, size );

        if ( NULL == pNode ) return NULL;
        pNode->sign     = MAKE_UNIT_NODE_MAGIC( pNode );
        pNode->offset   = (size_t)pNode - (size_t)(&s_unitManager);
        pNode->size     = nmemb * size;
        pNode->pData    = PTR_UNIT_NODE_DATA( pNode );
        pNode->pPrev    = NULL;
        pNode->pNext    = NULL;
        appendUnit( pNode );

#ifdef _DEBUG
        if ( !bRecursive )
            fprintf(stderr, "===calloc: %p, size: %ld, real: %ld\n", pNode, pNode->size, pNode->size - DEF_SIZE_UNIT_NODE);
#endif
        return PTR_UNIT_NODE_DATA( pNode );
    }

    void* CMemoryManager::_impRealloc( void *ptr, size_t size, bool bRecursive )
    {
        //if ( NULL != ptr ) _impFree( ptr, true );
        tagUnitNode* pNode = (tagUnitNode*)m_pRealRealloc( NULL, size + DEF_SIZE_UNIT_NODE );
        
        if ( NULL == pNode ) return NULL;
        pNode->sign     = MAKE_UNIT_NODE_MAGIC( pNode );
        pNode->offset   = (size_t)pNode - (size_t)(&s_unitManager);
        pNode->size     = size + DEF_SIZE_UNIT_NODE;
        pNode->pData    = PTR_UNIT_NODE_DATA( pNode );
        pNode->pPrev    = NULL;
        pNode->pNext    = NULL;
        appendUnit( pNode );

#ifdef _DEBUG
        if ( !bRecursive )
            fprintf(stderr, "===realloc: %p, size: %ld, real: %ld\n", pNode, pNode->size, pNode->size - DEF_SIZE_UNIT_NODE);
#endif
        return PTR_UNIT_NODE_DATA( pNode );
    }

    void* CMemoryManager::_impMemalign( size_t blocksize, size_t size, bool bRecursive )
    {
        tagUnitNode* pNode = (tagUnitNode*)m_pRealMemalign( blocksize, size + DEF_SIZE_UNIT_NODE );

        if ( NULL == pNode ) return NULL;
        pNode->sign     = MAKE_UNIT_NODE_MAGIC( pNode );
        pNode->offset   = (size_t)pNode - (size_t)(&s_unitManager);
        pNode->size     = size + DEF_SIZE_UNIT_NODE;
        pNode->pData    = PTR_UNIT_NODE_DATA( pNode );
        pNode->pPrev    = NULL;
        pNode->pNext    = NULL;
        appendUnit( pNode );

#ifdef _DEBUG
        if ( !bRecursive )
            fprintf(stderr, "===memalign: %p, size: %ld, real: %ld\n", pNode, pNode->size, pNode->size - DEF_SIZE_UNIT_NODE);
#endif
        return PTR_UNIT_NODE_DATA( pNode );
    }

    void* CMemoryManager::_impValloc( size_t size, bool bRecursive )
    {
        tagUnitNode* pNode = (tagUnitNode*)m_pRealValloc( size + DEF_SIZE_UNIT_NODE );

        if ( NULL == pNode ) return NULL;
        pNode->sign     = MAKE_UNIT_NODE_MAGIC( pNode );
        pNode->offset   = (size_t)pNode - (size_t)(&s_unitManager);
        pNode->size     = size + DEF_SIZE_UNIT_NODE;
        pNode->pData    = PTR_UNIT_NODE_DATA( pNode );
        pNode->pPrev    = NULL;
        pNode->pNext    = NULL;
        appendUnit( pNode );

#ifdef _DEBUG
        if ( !bRecursive )
            fprintf(stderr, "===valloc: %p, size: %ld, real: %ld\n", pNode, pNode->size, pNode->size - DEF_SIZE_UNIT_NODE);
#endif
        return PTR_UNIT_NODE_DATA( pNode );
    }

    void CMemoryManager::_impFree( void* ptr, bool bRecursive )
    {
        if ( NULL == ptr ) return;
        tagUnitNode* pNode = PTR_UNIT_NODE_HEADER( ptr );

#ifdef _DEBUG       
        if ( !bRecursive )
            fprintf(stderr, "===free: %p, size: %ld, real: %ld\n", pNode, pNode->size, pNode->size - DEF_SIZE_UNIT_NODE);
#endif        
        deleteUnit( pNode );
        m_pRealFree( pNode );
    }
}
