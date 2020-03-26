#include <string>
#include <cstdarg>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "NET_Core.hpp"

namespace NET
{
	void CLog::initLog(const char* name)
	{
		google::InitGoogleLogging(name);
		FLAGS_log_dir 			= "/tmp/log";
		FLAGS_alsologtostderr 	= true;
		FLAGS_colorlogtostderr 	= true;
		FLAGS_minloglevel 		= LL_INFO;

		checkDirection("/tmp/log");
	}

	void CLog::setDirection(const char* strPath)
	{
		if ( !checkDirection(strPath) ) return;
		
		FLAGS_log_dir = strPath;
	}

	void CLog::setDirection(LOG_LEVEL level, const char* strPath)
	{
		if ( !checkDirection(strPath) ) return;

		google::SetLogDestination(level, strPath);
	}

	void CLog::setFilter(LOG_LEVEL level)
	{
		FLAGS_minloglevel = level;
	}

	void CLog::enableToStderr(bool bEnable)
	{
		FLAGS_alsologtostderr = bEnable;
	}

	void CLog::enableColor(bool bEnable)
	{
		FLAGS_colorlogtostderr = bEnable;
	}

	std::string CLog::format(const char* fmt, ...) 
	{
		int size = 512;
		char* buff = new char[size];
		
		va_list v1;
		va_start(v1, fmt);
		int nsize = vsnprintf(buff, size, fmt, v1);
		if ( size <= nsize ) {
			delete[] buff;
			buff = new char[nsize + 1];
		
			nsize = vsnprintf(buff, size, fmt, v1);
		}
		std::string ret(buff);
		
		va_end(v1);
		delete[] buff;
		
		return ret;
	}

	bool CLog::checkDirection(const char* strPath)
	{
		DIR* dir = opendir(strPath);
		if ( dir ) {
			closedir(dir);
			return true;
		} else if ( ENOENT == errno ) {
			return ( 0 == mkdir(strPath, 0774) );
		} else {
			return false;
		}
	}
}
