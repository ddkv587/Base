/*
 * @Author: ddkV587 
 * @Date: 2020-03-18 17:48:55 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-18 17:55:33
 */

#ifndef __BASE_MACRODEFINE_HPP__
#define __BASE_MACRODEFINE_HPP__

#include "TypeDefine.hpp"

// ================= memory =================
#define MEMORY_CONTROL \
    void* operator new( size_t size ) \
    { \
        return ::Base::Memory::malloc(size); \
    } \
    void* operator new[]( size_t size ) \
    { \
        return ::Base::Memory::malloc( size ); \
    } \
    void operator delete[]( void* p ) \
    { \
        ::Base::Memory::free( p ); \
    } \
    void operator delete( void* p ) \
    { \
        ::Base::Memory::free( p ); \
    }

#define IMP_OPERATOR_NEW( ClassName ) \
    void* operator new( size_t size ) \
    { \
        static INT s_uiClassID = 0; \
        if ( s_uiClassID == 0 ) \
        { \
            s_uiClassID = ::Base::Memory::registClassName( ClassName ); \
        } \
        return ::Base::Memory::malloc( size, ClassName, s_uiClassID ); \
    } \
    void* operator new[]( size_t size ) \
    { \
        static INT s_uiClassID = 0; \
        if ( s_uiClassID == 0 ) \
        { \
            s_uiClassID = ::Base::Memory::registClassName( ClassName ); \
        } \
        return ::Base::Memory::malloc( size, ClassName, s_uiClassID ); \
    }
    /*
    //void operator delete(void* p) \
    //{ \
    //	static INT s_uiClassID = 0; \
    //	if ( s_uiClassID == 0 ) \
    //	{ \
    //		s_uiClassID = ::Base::Memory::registClassName( ClassName ); \
    //	} \
    //	return ::Base::Memory::delete( p, ClassName, s_uiClassID ); \
    //} \
    //void operator delete[](void* p) \
    //{ \
    //	static INT s_uiClassID = 0; \
    //	if ( s_uiClassID == 0 ) \
    //	{ \
    //		s_uiClassID = ::Base::Memory::registClassName( ClassName ); \
    //	} \
    //	return ::Base::Memory::delete( p, ClassName, s_uiClassID ); \
    //}
    */

#define CHECK_OBJECT_PTR( ObjPtr, Hint ) \
    { \
        ::Base::Memory::checkPtr( ObjPtr, Hint ); \
    }

#endif