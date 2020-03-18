/*
 * @Author: ddkV587 
 * @Date: 2020-03-18 17:14:46 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-18 17:16:44
 */

#ifndef __NET_CTHREADHPP__
#define __NET_CTHREADHPP__

namespace Base
{
    class CThreadPool;

    class CThread
    {
    public:// TODO: define const here
        enum EPolicy
        {
            SCHED_OTHER = 0,
            SCHED_BATCH,
            SCHED_IDLE,
            SCHED_FIFO,
            SCHED_RR
        };

    private:// TODO: define const here

    public:// TODO: define embed class or struct or enum here

    private:// TODO: define embed class or struct or enum here

    public:// TODO: define your public method here
        CThread();
        virtual ~CThread();

        void                            start( void* arg = NULL )                       { m_thread = ::std::thread(&CThread::mainLoop, this, std::move(arg)); }
        void                            stop()                                          { m_bStop = TRUE; }
        cosnt ::std::thread::id 		threadID()                                      { return m_thread.get_id(); }

        inline const STRING&            threadName()                                    { return m_strName; }
        void                            threadName( const STRING& strName )             { m_strName = strName; }

        // ============= priority ===================
        inline EPolicy                  policy()                                        { return m_ePolicy; }
        inline INT                      priority()                                      { return m_iPriority; }
        BOOLEAN                         priority( INT iPriority, EPolicy policy );

        // ============= affinity ===================
        //const UINT*                   affinity();
        BOOLEAN                         affinity( const UINT[]& cpus );

        // ============= Thread Pool ===================
        void                            assignPool( CThreadPool* pool );
        BOOLEAN                         releasePool();

    protected:// TODO: define your protected method here
        void                            wait()                                          { m_waitCondition.wait_for( ::std::unique_lock<std::mutex>( m_waitMutex ), std::chrono::milliseconds( 2000 ) ); }
        void                            notify()                                        { m_waitCondition.notify_one(); }

        virtual void                    mainLoop(void* arg);

    private:// TODO: define your private method here
        CThread(CThread&) = delete;
        CThread(const CThread&) = delete;
        CThread& operator=(const CThread&) = delete;

        inline INT                      transformPolicy( EPolicy policy );

    protected:// property
        BOOLEAN                         m_bStop;
        EPolicy                         m_ePolicy;
        UINT                            m_iPriority;

    private:// property
        STRING                          m_strName;
        STHREAD                         m_thread;

        SMUTEX                          m_poolMutex;
        CThreadPool*                    m_pAssignedPool;

        SMUTEX                          m_waitMutex;
        ::std::condition_variable       m_waitCondition;
    };
}

#endif  // __NET_CTHREADHPP__