#ifndef __LOGHPP__
#define __LOGHPP__

namespace NET
{
    class CLog 
    {
    public:

    public:
        enum LOG_LEVEL {
            LL_INFO,
            LL_WARNING,
            LL_ERROR,
            LL_FATAL,
        };

    public:
        static void 		initLog(const char*);
        static void 		setDirection(const char*);
        static void 		setDirection(LOG_LEVEL, const char*);
        static void 		setFilter(LOG_LEVEL);

        static void 		enableToStderr(bool);
        static void 		enableColor(bool);

        static std::string  format(const char* fmt, ...);

    protected:
        CLog() = delete;
        CLog(CLog&) = delete;
        ~CLog() {};

    private:
        static bool			checkDirection(const char* strPath);
    };
}
#endif
