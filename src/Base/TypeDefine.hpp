#ifndef __TYPEDEFINE_HPP__
#define __TYPEDEFINE_HPP__

#include <cstdint>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>

namespace NET
{
    typedef         char                            CHAR;
    typedef         int8_t                          INT8;
    typedef         uint8_t                         UINT8;
    typedef         int16_t                         INT16;
    typedef         uint16_t                        UINT16;
    typedef         int32_t                         INT;
    typedef         uint32_t                        UINT;
    typedef         int64_t                         INT64;
    typedef         uint64_t                        UINT64;

    typedef         bool                            BOOLEAN;
    typedef         float                           FLOAT;
    typedef         double                          DOUBLE;

    typedef         UINT8                           BYTE;
    typedef         INT64                           LONG;
    typedef         UINT64                          ULONG;
    typedef         INT64                           LLONG;
    typedef         UINT64                          ULLONG;
    typedef         size_t                          KEY;

    typedef         ::std::string                   STRING;
    typedef         ::std::thread                   STHREAD;
    typedef         ::std::mutex                    SMUTEX;
    typedef         ::std::condition_variable       SCONDITION;

    
    template< class T > using SVECTOR          = ::std::vector< T >;
    template< class T > using SLIST            = ::std::list< T >;
    template< class T > using SQUEUE           = ::std::queue< T >;
    template< class T > using SSTACK           = ::std::stack< T >;

    template< class Key, class Value, class Compare = ::std::less<Key> > 
    using SMAP             = ::std::map< Key, Value, Compare = ::std::less< Key > >;
    template< class R, class... Args >
    using SFUNCTION        = ::std::function< R( Args... ) >;

    #ifndef TRUE
    #define TRUE                        true
    #endif

    #ifndef FALSE
    #define FALSE                       false
    #endif

    #ifndef STRING_NULL
    #define STRING_NULL                 ""
    #endif

    #ifndef NULL
    #define NULL                        nullptr
    #endif

    #define UNUSED(X)                   (void)(X)

    #ifdef __DEBUG__
        #include <assert.h>
        #define assert_m( x, m )        do { UNUSED( m ); assert( x ); } while(0)
        #define assert_r( x, r )        do { UNUSED( r ); assert( x ); } while(0)
        #define assert_mr( x, m, r )    do { UNUSED( m ); UNUSED( r ); assert( x ); } while(0)
    
    #else
        #define assert( x ) \
                    do {\
                        if ( !( x ) ) \
                            return; \
                    } while(0)       
        #define assert_m( x, m ) \
                    do {\
                        if ( !( x ) ) {\
                            perror( m ); \
                            return; \
                        } \
                    } while(0)
        #define assert_r( x, r ) \
                    do {\
                        if ( !( x ) ) \
                            return ( r ); \
                    } while(0)          
        #define assert_mr( x, m, r ) \
                    do {\
                        if ( !( x ) ) {\
                            perror( m ); \
                            return ( r ); \
                        } \
                    } while(0)
                    
    #endif  // __ASSERT__
}

#endif
