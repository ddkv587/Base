#include <stdarg.h>
#include <glog/logging.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "CBase.hpp"

namespace Base
{
    void CLog::initLog( const STRING& strName )
    {
        ::google::InitGoogleLogging( strName );
        FLAGS_log_dir           = "/tmp/log";
        FLAGS_alsologtostderr   = true;
        FLAGS_colorlogtostderr  = true;
        FLAGS_minloglevel       = LL_INFO;

        checkDirection( "/tmp/log" );
    }

    void CLog::setDirection( const STRING& strPath )
    {
        if ( !checkDirection( strPath ) ) return;
        
        FLAGS_log_dir = strPath;
    }

    void CLog::setDirection( LOG_LEVEL level, const STRING& strPath )
    {
        if ( !checkDirection( strPath ) ) return;

        ::google::SetLogDestination( level, strPath );
    }

    void CLog::setFilter( LOG_LEVEL level )
    {
        FLAGS_minloglevel = level;
    }

    void CLog::enableToStderr( BOOLEAN bEnable )
    {
        FLAGS_alsologtostderr = bEnable;
    }

    void CLog::enableColor( BOOLEAN bEnable )
    {
        FLAGS_colorlogtostderr = bEnable;
    }

    std::string CLog::format( const STRING& fmt, ... ) 
    {
        INT iSize   = 512;
        CHAR* buff  = ( CHAR* )malloc( size );
        
        va_list v1;
        va_start(v1, fmt);
        INT nsize = vsnprintf( buff, size, fmt, v1 );
        if ( size <= nsize ) {
            // realloc
            free( buff );
            buff    = ( CHAR* )malloc( nsize + 1 );
            nsize   = vsnprintf( buff, size, fmt, v1 );
        }
        std::string ret( buff );
        va_end(v1);

        free( buff );
        return ret;
    }

    BOOLEAN CLog::checkDirection( const STRING& strPath )
    {
        DIR* dir = opendir( strPath );
        if ( dir ) {
            closedir( dir );
            return TRUE;
        } else if ( ENOENT == errno ) {
            return ( 0 == mkdir( strPath, 0774 ) );
        } else {
            return FALSE;
        }
    }
}