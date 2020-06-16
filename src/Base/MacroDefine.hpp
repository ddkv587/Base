/*
 * @Author: ddkV587 
 * @Date: 2020-03-18 17:48:55 
 * @Last Modified by: ddkV587
 * @Last Modified time: 2020-03-18 17:55:33
 */

#ifndef DEF__BASE_MACRODEFINE_HPP__
#define DEF__BASE_MACRODEFINE_HPP__

#include "TypeDefine.hpp"

namespace Base
{
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

    #ifndef MAX
    #define MAX(a, b)                   ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
    #endif

    #ifndef MIN
    #define MIN(a, b)                   ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
    #endif

    #ifndef SAFE_DELETE
    #define SAFE_DELETE(ptr)            do { if ( ( ptr ) ) { delete (ptr); (ptr) = NULL; } } while( 0 )

    #endif

// ================= memory =================
#define MEMORY_CONTROL \
    void* operator new( size_t size ) \
    { \
        return ::Base::Memory::traceMalloc( ::Base::SIZE( size ) ); \
    } \
    void* operator new[]( size_t size ) \
    { \
        return ::Base::Memory::traceMalloc( ::Base::SIZE( size ) ); \
    } \
    void operator delete[]( void* p ) \
    { \
        ::Base::Memory::traceFree( p ); \
    } \
    void operator delete( void* p ) \
    { \
        ::Base::Memory::traceFree( p ); \
    }

#define IMP_OPERATOR_NEW( ClassName ) \
    void* operator new( size_t size ) \
    { \
        static INT s_uiClassID = 0; \
        if ( s_uiClassID == 0 ) { \
            //s_uiClassID = ::Base::Memory::registClassName( ClassName );
        } \
        return ::Base::Memory::malloc( ::Base::SIZE( size ), ClassName, s_uiClassID ); \
    } \
    void* operator new[]( size_t size ) \
    { \
        static INT s_uiClassID = 0; \
        if ( s_uiClassID == 0 ) { \
            s_uiClassID = ::Base::Memory::registClassName( ClassName ); \
        } \
        return ::Base::Memory::malloc( ::Base::SIZE( size ), ClassName, s_uiClassID ); \
    }
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

#define CHECK_OBJECT_PTR( ObjPtr, Hint ) \
    { \
        ::Base::Memory::checkPtr( ObjPtr, Hint ); \
    }

}

#endif