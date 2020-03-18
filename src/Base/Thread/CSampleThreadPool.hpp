/*
 * @Author: ddkV587 
 * @Date: 2020-02-05 11:56:53 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-18 17:16:56
 */

#ifndef __BASE_CSAMPLETHREADMANAGERHPP__
#define __BASE_CSAMPLETHREADMANAGERHPP__

#include "TypeDefine.hpp"

namespace Base
{
    class CThreadPool
    {
        public:
            typedef void ( *PTRTASK )( void* );
            
        private:
            struct tagTask
            {
                explicit tagTask( ::std::function< void(void*) > func = NULL, void* argu = NULL )
                    : pFunc( func )
                    , pArgu( argu )
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