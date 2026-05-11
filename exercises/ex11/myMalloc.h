#ifndef _MY_MALLOC_H_
#define _MY_MALLOC_H_

#include <stdlib.h>

// Print out a string without using stdio (functions in that part of the
// C library seem to call malloc).
void reportString( const char * );

// Print out a report of all the holes (free block) on our free list.  This
// function doesn't use stdio (since it seems to call malloc)
void reportFreeList();

// The standard memory allocation functions.  We don't really need
// these, since they get declared in stdlib.h anyway. As long as the
// prototypes here are consistent with the ones from stdlib.h, the
// compiler should be happy.

void *malloc( size_t size );

void free( void *ptr );

void *calloc( size_t count, size_t size );

void *realloc( void *ptr, size_t size );

#endif
