#include "CBase.hpp"

namespace Base
{
    CThreadPool::CThreadPool( UINT uiThreadSize, UINT uiTaskSize )
        : m_bStop( FALSE )
        , m_bAvailable( FALSE )
        , m_uiThreadSize( uiThreadSize )
        , m_uiTaskSize( uiTaskSize )
    {
        // init thread queue
        for ( UINT ui = 0; ui < uiThreadSize; ++ui ) {
            m_threadVector.emplace_back( ::std::bind( &CThreadPool::innerLoop, this ) );
        }
        // init task queue
    }

    CThreadPool::~CThreadPool()
    {
        destroy();
    }

    void CThreadPool::start()
    {
        m_bAvailable = TRUE;
    }

    void CThreadPool::stop()
    {
        m_bAvailable = FALSE;
    }

    void CThreadPool::destroy()
    {
        if ( !m_bStop ) {
            m_bStop = TRUE;

            broadcast();

            for ( auto it = m_threadVector.begin(); it != m_threadVector.end(); ++it ) {
                
                (*it)->join();
            }

            ::std::lock_guard<SMUTEX> lk( m_taskMutex );

            SQUEUE<tagTask>().swap( m_taskQueue );
        }
    }

    BOOLEAN CThreadPool::addTask( TASK t, void* args )
    {
        if ( m_bAvailable && t )
            m_taskQueue.emplace( ::std::function<TASK>( t, ::std::placeholders::_1 ), args );

         notify();

        return TRUE;
    }

    void CThreadPool::innerLoop()
    {
        while ( !m_bStop ) {
            auto t = task();
            if ( t ) {
                t->operator( t->argument );
            } else {
                ::std::unique_lock<SMUTEX> ulock( m_threadMutex );
                m_treadCondition.wait( ulock, [this] { return ( m_bStop || ( !m_taskQueue.empty() ) ); } );
            }
        }

        printf( "thread %lld exit!\n", ::std::this_thread::get_id() );
    } 
    
    tagTask* CThreadPool::task()
    {
        ::std::lock_guard<SMUTEX> lk( m_taskMutex );

        tagTask* t = NULL;
        if ( !m_taskQueue.empty() ) {
            t = m_taskQueue.front();
            m_taskQueue.pop();
        }

        return t;
    }

    void CThreadPool::notify()
    {
        m_treadCondition.notify_one();
    }

    void CThreadPool::broadcast()
    {
        m_treadCondition.notify_all();
    }
}
