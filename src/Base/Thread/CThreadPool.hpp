/*
 * @Author: ddkV587 
 * @Date: 2020-02-05 11:56:31 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-18 17:15:00
 */

#ifndef __BASE_CTHREADPOOLHPP__
#define __BASE_CTHREADPOOLHPP__

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