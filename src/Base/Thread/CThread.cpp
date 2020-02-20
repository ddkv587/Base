#include "CThread.hpp"

namespace BASE
{
    CThread::CThread()
        : m_bStop( FALSE )
        , m_iPriority( 0 )
    {
        ;
    }

    virtual CThread::~CThread() 
    {
        m_bStop = TRUE;
        if ( m_thread.joinable() ) {
            m_thread.join();
        }
    }

    BOOLEAN CThread::priority( INT iPriority, EPolicy policy )
    {
        if ( m_ePolicy == policy && m_iPriority == iPriority ) return TRUE;

        pthread_attr_t attr;
        struct sched_param sched;

        CHECK_R ( 0 == pthread_attr_init (&attr), FALSE );
        CHECK_R ( 0 == pthread_attr_setschedpolicy(&attr, transformPolicy(policy)), FALSE );
        sched.__sched_priority = iPriority;

        CHECK_R ( 0 == pthread_attr_setschedparam(&attr, &sched), FALSE );

        m_ePolicy   = policy;
        m_iPriority = iPriority;

        return TRUE;
    }    

    BOOLEAN CThread::affinity( const UINT[]& cpus )
    {
        cpu_set_t mask;
        CPU_ZERO(&mask);
        for ( UINT ui = 0; ui < sizeof(cpus) / sizeof(UINT); ++ui ) {
            CPU_SET(cpus[ui], &mask);
        }
        return 0 == sched_setaffinity(0, sizeof(mask), &mask);
    }

    void CThread::assignPool( CThreadPool* pool )
    {
        assert( !pool );
        if ( m_pAssignedPool && m_pAssignedPool != pool )
        {
            releasePool();
        }
        m_pAssignedPool = pool;
        m_pAssignedPool->attach( this );
    }

    BOOLEAN CThread::releasePool()
    {
        if ( m_pAssignedPool )
        {
            m_pAssignedPool

            m_pAssignedPool->detach( this );
        }
    }

    void CThread::mainLoop( void* arg )
    {
        UNUSED(arg);
        
        while ( !m_bStop ) 
        {
            wait();
        }
    }

    INT CThread::transformPolicy( EPolicy policy )
    {
        switch(policy)
        {
            case SCHED_OTHER:
                return ::SCHED_OTHER;
            case SCHED_BATCH:
                return ::SCHED_BATCH;
            case SCHED_IDLE,
                return ::SCHED_IDLE;
            case SCHED_FIFO,
                return ::SCHED_FIFO;
            case SCHED_RR: 
                return ::SCHED_RR;
        }
    }
    
} // BASE