/*
 * syck.h
 *
 * $Author$
 * $Date$
 *
 * Copyright (C) 2003 why the lucky stiff
 */

#ifndef SYCK_H
#define SYCK_H

#define SYCK_VERSION    "0.35"
#define YAML_DOMAIN     "yaml.org,2002"

#include <stdio.h>
#ifdef HAVE_ST_H
#include <st.h>
#else
#include "syck_st.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * Memory Allocation
 */
#if defined(HAVE_ALLOCA_H) && !defined(__GNUC__)
#include <alloca.h>
#endif

#if DEBUG
  void syck_assert( char *, unsigned );
# define ASSERT(f) \
    if ( f ) \
        {}   \
    else     \
        syck_assert( __FILE__, __LINE__ )
#else
# define ASSERT(f)
#endif

#ifndef NULL
# define NULL (void *)0
#endif

#define ALLOC_CT 8
#define S_ALLOC_N(type,n) (type*)malloc(sizeof(type)*(n))
#define S_ALLOC(type) (type*)malloc(sizeof(type))
#define S_REALLOC_N(var,type,n) (var)=(type*)realloc((char*)(var),sizeof(type)*(n))
#define S_FREE(n) free(n); n = NULL;

#define S_ALLOCA_N(type,n) (type*)alloca(sizeof(type)*(n))

#define S_MEMZERO(p,type,n) memset((p), 0, sizeof(type)*(n))
#define S_MEMCPY(p1,p2,type,n) memcpy((p1), (p2), sizeof(type)*(n))
#define S_MEMMOVE(p1,p2,type,n) memmove((p1), (p2), sizeof(type)*(n))
#define S_MEMCMP(p1,p2,type,n) memcmp((p1), (p2), sizeof(type)*(n))

#define BLOCK_FOLD  10
#define BLOCK_LIT   20
#define BLOCK_PLAIN 30
#define NL_CHOMP    130
#define NL_KEEP     140

/*
 * Node definitions
 */
#define SYMID unsigned long

typedef struct _syck_parser SyckParser;
typedef struct _syck_file SyckIoFile;
typedef struct _syck_str SyckIoStr;
typedef struct _syck_node SyckNode;
typedef struct _syck_level SyckLevel;

enum syck_kind_tag {
    syck_map_kind,
    syck_seq_kind,
    syck_str_kind
};

enum map_part {
    map_key,
    map_value
};

struct _syck_node {
    // Symbol table ID
    SYMID id;
    // Underlying kind
    enum syck_kind_tag kind;
    // Fully qualified tag-uri for type
    char *type_id;
    // Anchor name
    char *anchor;
    union {
        // Storage for map data
        struct SyckMap {
            SYMID *keys;
            SYMID *values;
            long capa;
            long idx;
        } *pairs;
        // Storage for sequence data
        struct SyckSeq {
            SYMID *items;
            long capa;
            long idx;
        } *list;
        // Storage for string data
        struct SyckStr {
            char *ptr;
            long len;
        } *str;
    } data;
    // Shortcut node
    void *shortcut;
};

/*
 * Parser definitions
 */
typedef SYMID (*SyckNodeHandler)(SyckParser *, SyckNode *);
typedef void (*SyckErrorHandler)(SyckParser *, char *);
typedef long (*SyckIoFileRead)(char *, SyckIoFile *, long, long); 
typedef long (*SyckIoStrRead)(char *, SyckIoStr *, long, long);

enum syck_io_type {
    syck_io_str,
    syck_io_file
};

enum syck_level_status {
    syck_lvl_header,
    syck_lvl_doc,
    syck_lvl_seq,
    syck_lvl_map,
    syck_lvl_block,
    syck_lvl_str,
    syck_lvl_inline,
    syck_lvl_end,
    syck_lvl_pause
};

struct _syck_parser {
    // Root node
    SYMID root, root_on_error;
    // Implicit typing flag
    int implicit_typing, taguri_expansion;
    // Scripting language function to handle nodes
    SyckNodeHandler handler;
    // Error handler
    SyckErrorHandler error_handler;
    // IO type
    enum syck_io_type io_type;
    // Custom buffer size
    size_t bufsize;
    // Buffer pointers
    char *buffer, *linectptr, *lineptr, *toktmp, *token, *cursor, *marker, *limit;
    // Line counter
    int linect;
    // Last token from yylex()
    int last_token;
    // Force a token upon next call to yylex()
    int force_token;
    // EOF flag
    int eof;
    union {
        struct _syck_file {
            FILE *ptr;
            SyckIoFileRead read;
        } *file;
        struct _syck_str {
            char *beg, *ptr, *end;
            SyckIoStrRead read;
        } *str;
    } io;
    // Symbol table
    st_table *anchors;
    // Optional symbol table for SYMIDs
    st_table *syms;
    // Levels of indentation
    struct _syck_level {
        int spaces;
        char *domain;
        enum syck_level_status status;
    } *levels;
    int lvl_idx;
    int lvl_capa;
    void *bonus;
};

/*
 * Handler prototypes
 */
SYMID syck_hdlr_add_node( SyckParser *, SyckNode * );
SyckNode *syck_hdlr_add_anchor( SyckParser *, char *, SyckNode * );
SyckNode *syck_hdlr_add_alias( SyckParser *, char * );
void syck_add_transfer( char *, SyckNode *, int );
char *syck_xprivate( char *, int );
char *syck_taguri( char *, char *, int );
int syck_add_sym( SyckParser *, char * );
int syck_lookup_sym( SyckParser *, SYMID, char ** );
int syck_try_implicit( SyckNode * );
char *syck_type_id_to_uri( char * );
void try_tag_implicit( SyckNode *, int );
char *syck_match_implicit( char *, size_t );

/*
 * API prototypes
 */
char *syck_strndup( char *, long );
long syck_io_file_read( char *, SyckIoFile *, long, long );
long syck_io_str_read( char *, SyckIoStr *, long, long );
SyckParser *syck_new_parser();
void syck_free_parser( SyckParser * );
void syck_parser_set_root_on_error( SyckParser *, SYMID );
void syck_parser_implicit_typing( SyckParser *, int );
void syck_parser_taguri_expansion( SyckParser *, int );
void syck_parser_handler( SyckParser *, SyckNodeHandler );
void syck_parser_error_handler( SyckParser *, SyckErrorHandler );
void syck_parser_file( SyckParser *, FILE *, SyckIoFileRead );
void syck_parser_str( SyckParser *, char *, long, SyckIoStrRead );
void syck_parser_str_auto( SyckParser *, char *, SyckIoStrRead );
SyckLevel *syck_parser_current_level( SyckParser * );
void syck_parser_add_level( SyckParser *, int, enum syck_level_status );
void syck_parser_pop_level( SyckParser * );
void free_any_io( SyckParser * );
long syck_parser_read( SyckParser * );
long syck_parser_readlen( SyckParser *, long );
void syck_parser_init( SyckParser *, int );
SYMID syck_parse( SyckParser * );
void syck_default_error_handler( SyckParser *, char * );

/*
 * Allocation prototypes
 */
SyckNode *syck_alloc_map();
SyckNode *syck_alloc_seq();
SyckNode *syck_alloc_str();
void syck_free_node( SyckNode * );
void syck_free_members( SyckNode * );
SyckNode *syck_new_str( char * );
SyckNode *syck_new_str2( char *, long );
void syck_str_blow_away_commas( SyckNode * );
char *syck_str_read( SyckNode * );
SyckNode *syck_new_map( SYMID, SYMID );
void syck_map_add( SyckNode *, SYMID, SYMID );
SYMID syck_map_read( SyckNode *, enum map_part, long );
void syck_map_assign( SyckNode *, enum map_part, long, SYMID );
long syck_map_count( SyckNode * );
void syck_map_update( SyckNode *, SyckNode * );
SyckNode *syck_new_seq( SYMID );
void syck_seq_add( SyckNode *, SYMID );
SYMID syck_seq_read( SyckNode *, long );
long syck_seq_count( SyckNode * );

void apply_seq_in_map( SyckParser *, SyckNode * );

#ifndef ST_DATA_T_DEFINED
typedef long st_data_t;
#endif

#if defined(__cplusplus)
}  /* extern "C" { */
#endif

#endif /* ifndef SYCK_H */
