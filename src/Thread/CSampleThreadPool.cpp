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
            m_threadVector.emplace( new STHREAD( ::std::bind( &CThreadPool::innerLoop, this ) ) );
        }
        // init task queue

        m_bAvailable = TRUE;
    }

    CThreadPool::~CThreadPool()
    {
        ;
    }

    void CThreadPool::start()
    {
        ;
    }

    void CThreadPool::stop()
    {
        if ( !m_bStop ) {
            m_bStop = TRUE;

            broadcast();

            for ( auto it = m_threadVector.begin(); it != m_threadVector.end(); ++it ) {
                (*it)->join();
            }
        }
    }

    void CThreadPool::destroy()
    {
        ;
    }

    BOOLEAN CThreadPool::addTask( TASK t )
    {
        if ( t ) 
            m_taskQueue.push_back( t );

        notify();
    }

    void CThreadPool::innerLoop()
    {
        while ( !m_bStop ) {
            auto t = parent->task();
            if ( t ) {
                t();
            } else {
                m_treadCondition.wait( new ::std::unique_lock<SMUTEX>( m_threadMutex ), [] { return !m_taskQueue.empty(); } );
            }
        }
    } 
    
    TASK CThreadPool::task()
    {
        ::std::lock_guard<SMUTEX> lk( m_taskMutex );

        TASK t = NULL;
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
