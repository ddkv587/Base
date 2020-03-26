/*
 * @Author: ddkV587 
 * @Date: 2020-03-26 14:56:01 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-26 15:35:09
 */

#ifndef __BASE_CLOG_HPP__
#define __BASE_CLOG_HPP__

namespace Base
{
    class CLog final
    {
    public:// const define

    private:// const define

    public:// embed class or struct or enum
        enum LOG_LEVEL {
            LL_INFO,
            LL_WARNING,
            LL_ERROR,
            LL_FATAL,
        };

    private:// embed class or struct or enum

    public:// method
        static void         initLog( const STRING& );
        static void         setDirection( const STRING& );
        static void         setDirection( LOG_LEVEL, const STRING& );
        static void         setFilter( LOG_LEVEL );

        static void         enableToStderr( BOOLEAN );
        static void         enableColor( BOOLEAN );

        static STRING       format( const STRING&  fmt, ... );

    protected:
        CLog() = delete;
        CLog( CLog& ) = delete;
        CLog( CLog&& ) = delete;
        ~CLog() {};

    private:
        static BOOLEAN      checkDirection( const STRING& strPath );
    };
}
#endif
