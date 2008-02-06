/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************
 * Simple class that caches the sounds played - part of jukebox
 *****************************************************************************/

#include <assert.h>
#include "sample_cache.h"
#include "tool/debug.h"

SampleCache::SampleCache( size_t memory_limit )
    : m_memory_limit( memory_limit )
    , m_used_memory( 0 )
{
}

bool SampleCache::CacheIfPossible( CachedChunk & chk,
                                   const std::string & file_name )
{
    assert( NULL == chk.m_chunk );
    assert( 0 == chk.m_refcount );

    // we need this to know the length
    FILE * f = fopen( file_name.c_str(), "rb" );
    if ( NULL == f )
        return false;

    fseek( f, 0, SEEK_END );
    // Size for sample should fit into 32 bits even if return is 64bits
    chk.m_size = ftell( f );
    fclose( f );

    if ( m_memory_limit > 0 && chk.m_size + m_used_memory > m_memory_limit )
        return false; // refuse to load when cache is full

    chk.m_chunk = Mix_LoadWAV( file_name.c_str() );
    if ( NULL == chk.m_chunk )
        return false;

    m_used_memory += chk.m_size;
    chk.m_times_used = 0;
    chk.m_filename = file_name;

    MSG_DEBUG( "jukebox.cache",
               "caching sample '%s' (size %uB), total cache size: %uB",
               chk.m_filename.c_str(), chk.m_size, m_used_memory );

    return true;
};

Mix_Chunk * SampleCache::LoadSound( const std::string & file_name )
{
    // look it up first
    int slot = FindChunkByName( file_name );

    if ( -1 == slot )
    {
        // not found, so cache it
        CachedChunk chk;
        if ( CacheIfPossible( chk, file_name ) )
        {
            m_cache.push_back( chk );
            slot = m_cache.size() - 1;
            m_chunks_by_name.insert( std::make_pair( file_name, slot ) );
            m_chunks_by_addr.insert( std::make_pair( chk.m_chunk, slot ) );
        }
    }

    if ( -1 != slot )
    {
        CachedChunk & chk = m_cache[ slot ];
        chk.m_times_used++;
        chk.m_refcount ++;
        return chk.m_chunk;
    }

    // we only get here if we couldn't cache
    // so delegate the work to mixer
    return Mix_LoadWAV( file_name.c_str() );
}

void SampleCache::FreeChunk( Mix_Chunk * pchk )
{
    int slot = FindChunkByAddr( pchk );

    if ( -1 != slot )
    {
        CachedChunk & chk = m_cache[ slot ];
        chk.m_refcount--;
        assert( chk.m_refcount >= 0 );
        // we don't delete the chunk when refcount == 0, because it can
        // defeat the whole idea of caching (imagine what if game
        // keeps loading and freeing the same sound)
        return;
    }

    // if chunk cannot be found, it's probably because we didn't cache it
    // delegate the work to mixer
    Mix_FreeChunk( pchk );
}

void SampleCache::Clear()
{
    for ( std::vector< CachedChunk >::iterator iter = m_cache.begin();
          iter != m_cache.end(); iter ++ )
    {
        CachedChunk & chk = *iter;

        m_used_memory -= chk.m_size;
        if ( chk.m_refcount != 0 )
        {
            MSG_DEBUG( "jukebox.cache",
                       "Freeing sample '%s' with refcount = %d",
                       chk.m_filename.c_str(), chk.m_refcount );
        }

        // print statistics
        MSG_DEBUG( "jukebox.cache", "sample '%s' used %d times",
                   chk.m_filename.c_str(), chk.m_times_used );

        Mix_FreeChunk( chk.m_chunk );

        // reinit
        chk.m_chunk = NULL;
        chk.m_refcount = 0;
        chk.m_times_used = 0;
        chk.m_size = 0;
        chk.m_filename = "";
    }

    m_cache.clear();

    m_chunks_by_name.clear();
    m_chunks_by_addr.clear();

    assert( 0 == m_used_memory );
}

int SampleCache::FindChunkByName( const std::string & name )
{
    std::map< std::string, int >::iterator it = m_chunks_by_name.find( name );
    if ( m_chunks_by_name.end() == it )
        return -1;

    return it->second;
}

int SampleCache::FindChunkByAddr( Mix_Chunk * pchk )
{
    std::map< Mix_Chunk *, int >::iterator it = m_chunks_by_addr.find( pchk );
    if ( m_chunks_by_addr.end() == it )
        return -1;

    return it->second;
}

