/*
 * @Author: ddkV587 
 * @Date: 2020-02-05 11:56:53 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-02-05 21:12:00
 */
#ifndef __CSAMPLETHREADMANAGERHPP__
#define __CSAMPLETHREADMANAGERHPP__

namespace Base
{
    class CThreadPool
    {
        private:
            typedef SFUNCTION< void(void*) >    TASK;   

        public:
            void                    start();
            void                    stop();
            void                    destroy();

            BOOLEAN                 addTask( TASK );
            
            CThreadPool( UINT uiThreadSize, UINT uiTaskSize = 100 );
            virtual ~CThreadPool();

        protected:
            CThreadPool( CThreadPool& ) = delete;
            CThreadPool( CThreadPool&& ) = delete;

        private:
            void                    innerLoop(  );
            TASK                    task();

            void                    notify();
            void                    broadcast();

        private:
            BOOLEAN                                 m_bStop;
            BOOLEAN                                 m_bAvailable;

            UINT                                    m_uiThreadSize;
            SMUTEX                                  m_threadMutex;
            SCONDITION                              m_treadCondition;
            SVECTOR< STHREAD >                      m_threadVector;

            UINT                                    m_uiTaskSize;
            SMUTEX                                  m_taskMutex;
            SQUEUE< TASK >                          m_taskQueue;
    };
} //Base
#endif