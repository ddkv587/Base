#ifndef __CSAMPLETHREADMANAGERHPP__
#define __CSAMPLETHREADMANAGERHPP__

namespace Base
{
    class CThreadPool
    {
        private:

        public:
            void                    start();
            void                    stop();
            void                    destroy();

            BOOLEAN                 addTask( SFUNCTION< void* > );

            CThreadPool( UINT uiThreadSize, UINT uiTaskSize = 100 );
            virtual ~CThreadPool();
        
        private:
            void                    notify();
            void                    broadcast();

        private:
            BOOLEAN                                 m_bStop;
            BOOLEAN                                 m_bAvailable;

            UINT                                    m_uiThreadSize;
            SMUTEX                                  m_threadMutex;
            SCONDITION                              m_treadCondition;
            SQUEUE< CThread >                       m_threadQueue;

            UINT                                    m_uiTaskSize;
            SMUTEX                                  m_taskMutex;
            SQUEUE< SFUNCTION< void* > >            m_taskQueue;
    };
} //Base
#endif