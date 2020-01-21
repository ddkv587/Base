#ifndef __CThreadManagerHPP__
#define __CThreadManagerHPP__

namespace NET
{
    class ITask
    {
        virtual void        run() = 0;
    };

    class CThreadPool
    {
        public:
            CThreadPool( UINT uiSize );
            virtual ~CThreadPool();

            

        protected:
           
        protected:
            SQUEUE< ITask > m_qeTask;
    };
} //NET
#endif