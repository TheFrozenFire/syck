//
// node.c
//
// $Author$
// $Date$
//
// Copyright (C) 2003 why the lucky stiff
//

#include "syck.h"

//
// Node allocation functions
//
SyckNode *
syck_alloc_node( enum syck_kind_tag type )
{
    SyckNode *s;

    s = S_ALLOC( SyckNode );
    s->kind = type;
    s->id = NULL;

    return s;
}

void
syck_free_node( SyckNode *n )
{
    syck_free_members( n );
    free( n );
}

SyckNode *
syck_alloc_map()
{
    SyckNode *n;
    struct SyckMap *m;

    m = S_ALLOC( struct SyckMap );
    m->idx = 0;
    m->capa = ALLOC_CT;
    m->keys = S_ALLOC_N( SYMID, m->capa );
    m->values = S_ALLOC_N( SYMID, m->capa );

    n = syck_alloc_node( syck_map_kind );
    n->data.pairs = m;
    
    return n;
}

SyckNode *
syck_alloc_seq()
{
    SyckNode *n;
    struct SyckSeq *s;

    s = S_ALLOC( struct SyckSeq );
    s->idx = 0;
    s->capa = ALLOC_CT;
    s->items = S_ALLOC_N( SYMID, s->capa );

    n = syck_alloc_node( syck_seq_kind );
    n->data.list = s;

    return n;
}

SyckNode *
syck_alloc_str()
{
    SyckNode *n;
    struct SyckStr *s;

    s = S_ALLOC( struct SyckStr );
    s->len = 0;
    s->ptr = NULL;

    n = syck_alloc_node( syck_str_kind );
    n->data.str = s;
    
    return n;
}

SyckNode *
syck_new_str( char *str )
{
    return syck_new_str2( str, strlen( str ) );
}

SyckNode *
syck_new_str2( char *str, long len )
{
    SyckNode *n;

    n = syck_alloc_str();
    n->data.str->ptr = S_ALLOC_N( char, len );
    n->data.str->len = len;
    memcpy( n->data.str->ptr, str, len );

    return n;
}

char *
syck_str_read( SyckNode *n )
{
    ASSERT( n != NULL );
    return n->data.str->ptr;
}

SyckNode *
syck_new_map( SYMID key, SYMID value )
{
    SyckNode *n;

    n = syck_alloc_map();
    syck_map_add( n, key, value );

    return n;
}

void
syck_map_add( SyckNode *map, SYMID key, SYMID value )
{
    struct SyckMap *m;
    long idx;

    ASSERT( map != NULL );
    ASSERT( map->data.pairs != NULL );
    
    m = map->data.pairs;
    idx = m->idx;
    m->idx += 1;
    if ( m->idx > m->capa )
    {
        m->capa += ALLOC_CT;
        S_REALLOC_N( m->keys, SYMID, m->capa );
        S_REALLOC_N( m->values, SYMID, m->capa );
    }
    m->keys[idx] = key;
    m->values[idx] = value;
}

void
syck_map_update( SyckNode *map1, SyckNode *map2 )
{
    struct SyckMap *m1, *m2;
    long new_idx, new_capa;
    ASSERT( map1 != NULL );
    ASSERT( map2 != NULL );

    m1 = map1->data.pairs;
    m2 = map2->data.pairs;
    if ( m2->idx < 1 ) return;
        
    new_idx = m1->idx;
    new_idx += m2->idx;
    new_capa = m1->capa;
    while ( new_idx > new_capa )
    {
        new_capa += ALLOC_CT;
    }
    if ( new_capa > m1->capa )
    {
        m1->capa = new_capa;
        S_REALLOC_N( m1->keys, SYMID, m1->capa );
        S_REALLOC_N( m1->values, SYMID, m1->capa );
    }
    new_idx = 0;
    for ( new_idx = 0; new_idx < m2->idx; m1->idx++, new_idx++ )
    {
        m1->keys[m1->idx] = m2->keys[new_idx]; 
        m1->values[m1->idx] = m2->values[new_idx]; 
    }
}

long
syck_map_count( SyckNode *map )
{
    ASSERT( map != NULL );
    ASSERT( map->data.pairs != NULL );
    return map->data.pairs->idx;
}

SYMID
syck_map_read( SyckNode *map, enum map_part p, long idx )
{
    struct SyckMap *m;

    ASSERT( map != NULL );
    m = map->data.pairs;
    ASSERT( m != NULL );
    if ( p == map_key )
    {
        return m->keys[idx];
    }
    else
    {
        return m->values[idx];
    }
}

SyckNode *
syck_new_seq( SYMID value )
{
    SyckNode *n;

    n = syck_alloc_seq();
    syck_seq_add( n, value );

    return n;
}

void
syck_seq_add( SyckNode *arr, SYMID value )
{
    struct SyckSeq *s;
    long idx;

    ASSERT( arr != NULL );
    ASSERT( arr->data.list != NULL );
    
    s = arr->data.list;
    idx = s->idx;
    s->idx += 1;
    if ( s->idx > s->capa )
    {
        s->capa += ALLOC_CT;
        S_REALLOC_N( s->items, SYMID, s->capa );
    }
    s->items[idx] = value;
}

long
syck_seq_count( SyckNode *seq )
{
    ASSERT( seq != NULL );
    ASSERT( seq->data.list != NULL );
    return seq->data.list->idx;
}

SYMID
syck_seq_read( SyckNode *seq, long idx )
{
    struct SyckSeq *s;

    ASSERT( seq != NULL );
    s = seq->data.list;
    ASSERT( s != NULL );
    return s->items[idx];
}

void
syck_free_members( SyckNode *n )
{
    int i;
    switch ( n->kind  )
    {
        case syck_str_kind:
            if ( n->data.str->ptr != NULL ) 
            {
                free( n->data.str->ptr );
                n->data.str->ptr = NULL;
                n->data.str->len = 0;
            }
        break;

        case syck_seq_kind:
            free( n->data.list->items );
            free( n->data.list );
        break;

        case syck_map_kind:
            free( n->data.pairs->keys );
            free( n->data.pairs->values );
            free( n->data.pairs );
        break;
    }
}

