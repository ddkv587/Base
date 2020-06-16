/*
 * @Author: ddkV587 
 * @Date: 2020-03-18 17:15:04 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-23 17:14:12
 */

#ifndef DEF__BASE_TYPEDEFINE_HPP__
#define DEF__BASE_TYPEDEFINE_HPP__

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
    typedef         uintptr_t                       UINTPTR;

    typedef         UINT8                           BYTE;
    typedef         INT64                           LONG;
    typedef         UINT64                          ULONG;
    typedef         INT64                           LLONG;
    typedef         UINT64                          ULLONG;
    
    typedef         ::std::string                   STRING;
    typedef         ::std::thread                   STHREAD;
    typedef         ::std::thread::id               STHREAD_ID;
    typedef         ::std::mutex                    SMUTEX;
    typedef         ::std::condition_variable       SCONDITION;

    template< class T > using SVECTOR          = ::std::vector< T >;
    template< class T > using SLIST            = ::std::list< T >;
    template< class T > using SQUEUE           = ::std::queue< T >;
    template< class T > using SSTACK           = ::std::stack< T >;
    template< class Key, class Value, class Compare = ::std::less<Key> > 
    using SMAP = ::std::map< Key, Value, Compare >;
}

#endif
