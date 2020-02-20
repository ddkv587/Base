#ifndef __CHUNKHPP__
#define __CHUNKHPP__

namespace NET
{
    class Chunk
    {
    public:
        Chunk( UINT64 size )
            : m_next( NULL )
            , m_prev( NULL ) 
            , m_userDataSize( size )
            , m_free( TRUE )
        {
            ;
        }

        void write( void* dest )
        {
            ::memcpy( dest, this, sizeof(Chunk) );
        }

        void read( void* src )
        {
            ::memcpy( this, src, sizeof(Chunk) );
        }

        Chunk*      m_next;
        Chunk*      m_prev;
        UINT64      m_userDataSize;
        BOOLEAN     m_free;
    };
}

#endif