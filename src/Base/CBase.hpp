/*
 * @Author: ddkV587 
 * @Date: 2020-03-18 17:14:38 
 * @Last Modified by:   ddkV587 
 * @Last Modified time: 2020-03-18 17:14:38 
 */

#ifndef __CBASE_HPP__
#define __CBASE_HPP__

namespace Base
{
    class CMemAllocator;
    class CMemManager;
    class Memory;

    class CThreadPool;
}

#include <atomic>
#include "TypeDefine.hpp"
#include "MacroDefine.hpp"

#include "Memory/Pool/CMemAllocator.hpp"
#include "Memory/Pool/CMemManager.hpp"
#include "Memory/Memory.hpp"

#include "Thread/CSampleThreadPool.hpp"

#endif