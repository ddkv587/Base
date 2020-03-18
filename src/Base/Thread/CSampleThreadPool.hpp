/*
 * @Author: ddkV587 
 * @Date: 2020-02-05 11:56:53 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-02-05 21:12:00
 */
#ifndef __CSAMPLETHREADMANAGERHPP__
#define __CSAMPLETHREADMANAGERHPP__

#include "TypeDefine.hpp"

namespace Base
{
    typedef void ( *PTRTASK )( void* );

    class CThreadPool
    {
        private:
            struct tagTask
            {
                tagTask()
                    : pFunc( NULL )
                    , pArgu( NULL )
                {
                    ;
                }

                BOOLEAN valid()
                {
                    return ( NULL != pFunc );
                }

                ::std::function< void(void*) >          pFunc;
                void*                                   pArgu;
            };

        public:
            void                    start();
            void                    stop();
            void                    destroy();

            BOOLEAN                 addTask( PTRTASK, void* );

            CThreadPool( UINT uiThreadSize, UINT uiTaskSize = 100 );
            virtual ~CThreadPool();

        protected:
            CThreadPool( CThreadPool& ) = delete;
            CThreadPool( CThreadPool&& ) = delete;

        private:
            void                            innerLoop();
            tagTask                         task();

            void                            notify();
            void                            broadcast();

        private:
            BOOLEAN                                     m_bStop;
            BOOLEAN                                     m_bAvailable;

            UINT                                        m_uiThreadSize;
            SMUTEX                                      m_threadMutex;
            SCONDITION                                  m_treadCondition;
            SVECTOR< STHREAD* >                         m_threadVector;

            UINT                                        m_uiTaskSize;
            SMUTEX                                      m_taskMutex;
            SQUEUE< tagTask >                           m_taskQueue;
    };
} //Base
#endif