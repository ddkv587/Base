/*
 * @Author: ddkV587 
 * @Date: 2020-03-23 16:08:30 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-26 15:51:40
 */

#ifndef __BASE_THIRDLIBRARY_HPP__
#define __BASE_THIRDLIBRARY_HPP__

// json library: https://github.com/nlohmann/json/tree/master
#include "Format/Json/nlohmann/json.hpp"
#include <glog/logging.h>

namespace Base
{
    using JSON  = ::nlohmann::json;
};

#endif