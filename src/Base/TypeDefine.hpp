/*
 * @Author: ddkV587 
 * @Date: 2020-03-18 17:15:04 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-23 16:26:35
 */

#ifndef __BASE_TYPEDEFINE_HPP__
#define __BASE_TYPEDEFINE_HPP__

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

#include "ThirdLibrary.hpp"

namespace Base
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
    typedef         size_t                          SIZE;

    typedef         UINT8                           BYTE;
    typedef         INT64                           LONG;
    typedef         UINT64                          ULONG;
    typedef         INT64                           LLONG;
    typedef         UINT64                          ULLONG;
    
    typedef         ::std::string                   STRING;
    typedef         ::std::thread                   STHREAD;
    typedef         ::std::mutex                    SMUTEX;
    typedef         ::std::condition_variable       SCONDITION;

    template< class T > using SVECTOR          = ::std::vector< T >;
    template< class T > using SLIST            = ::std::list< T >;
    template< class T > using SQUEUE           = ::std::queue< T >;
    template< class T > using SSTACK           = ::std::stack< T >;
    template< class Key, class Value, class Compare = ::std::less<Key> > 
    using SMAP             = ::std::map< Key, Value, Compare >;

    #ifndef TRUE
    #define TRUE                        true
    #endif

    #ifndef FALSE
    #define FALSE                       false
    #endif

    #ifndef NULL
    #define NULL                        nullptr
    #endif

    #ifndef STRING_NULL
    #define STRING_NULL                 ""
    #endif

    #ifndef UNUSED
    #define UNUSED(X)                   (void)(X)
    #endif

    // #ifdef __DEBUG__
    //     #include <assert.h>
    //     #define assert_m( x, m )        do { UNUSED( m ); assert( x ); } while(0)
    //     #define assert_r( x, r )        do { UNUSED( r ); assert( x ); } while(0)
    //     #define assert_mr( x, m, r )    do { UNUSED( m ); UNUSED( r ); assert( x ); } while(0)
    
    // #else
    //     #define assert( x ) \
    //                 do {\
    //                     if ( !( x ) ) \
    //                         return; \
    //                 } while(0)       
    //     #define assert_m( x, m ) \
    //                 do {\
    //                     if ( !( x ) ) {\
    //                         perror( m ); \
    //                         return; \
    //                     } \
    //                 } while(0)
    //     #define assert_r( x, r ) \
    //                 do {\
    //                     if ( !( x ) ) \
    //                         return ( r ); \
    //                 } while(0)
    //     #define assert_mr( x, m, r ) \
    //                 do {\
    //                     if ( !( x ) ) {\
    //                         perror( m ); \
    //                         return ( r ); \
    //                     } \
    //                 } while(0)
                    
    // #endif  // __ASSERT__
}

#endif
