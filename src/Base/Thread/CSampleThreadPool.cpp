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

            SQUEUE< ::std::function< void(void*) > >().swap(m_taskQueue);
        }
    }

    BOOLEAN CThreadPool::addTask( PTRTASK t, void* args )
    {
        if ( m_bAvailable && t )
            m_taskQueue.emplace( ::std::function< void(void*) >( t, std::placeholders::_1 ) );

        notify();

        return TRUE;
    }

    void CThreadPool::innerLoop()
    {
        while ( !m_bStop ) {
            auto t = task();
            if ( t ) {
                t->();
            } else {
                ::std::unique_lock<SMUTEX> ulock( m_threadMutex );
                m_treadCondition.wait( ulock, [this] { return ( m_bStop || ( !m_taskQueue.empty() ) ); } );
            }
        }

        printf( "thread %lld exit!\n", ::std::this_thread::get_id() );
    } 
    
    PTRTASK CThreadPool::task()
    {
        ::std::lock_guard<SMUTEX> lk( m_taskMutex );

        if ( !m_taskQueue.empty() ) {
            auto t = m_taskQueue.front();
            m_taskQueue.pop();

            return t;
        }

        return NULL;
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
