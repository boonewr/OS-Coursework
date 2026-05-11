#include <stdio.h>
#include <stdlib.h>
#include "myMalloc.h"

// A simple test, one alloc and one free.
void babyTest() {
  // Get a block that requires exactly one page.
  char *p = malloc( 4088 );

  // This should use up exactly one page on the common platform, so
  // the free list should be empty.
  reportString( "-------- babyTest 1 --------\n" );
  reportFreeList();

  // When we give back this memory, we should have just one block on the
  // free list.
  free( p );
  reportString( "-------- babyTest 2 --------\n" );
  reportFreeList();
}

// A larger test, multiple allocs with block splitting and merging.
void toddlerTest() {
  // Get 3 blocks that will all get carved out of the one block allocated in
  // babyTest().
  char *p1 = malloc( 30 );
  char *p2 = malloc( 50 );
  char *p3 = malloc( 20 );
  
  // There should still be some of that block remaining.
  reportString( "-------- toddlerTest 1 --------\n" );
  reportFreeList();

  // This block shouldn't merge with anything.
  free( p2 );
  reportString( "-------- toddlerTest 2 --------\n" );
  reportFreeList();

  // This block should merge with the one we just freed.
  free( p1 );
  reportString( "-------- toddlerTest 3 --------\n" );
  reportFreeList();

  // This block should merge with the two that were previously freed, so 
  // we should be back to just one 4088-byte free block.
  free( p3 );
  reportString( "-------- toddlerTest 4 --------\n" );
  reportFreeList();
}

#define TEST_SIZE 500
#define TEST_DURATION 10000

// A random test, to see if anything else goes wrong.
void randomTest() {
  char *block[ TEST_SIZE ] = {};

  for ( int i = 0; i < TEST_DURATION; i++ ) {
    // Choose a block to allocate or free.
    int ind = random() % TEST_SIZE;

    // allocate a block at this index if it's not already allocated.
    // Free it if it is.
    if ( block[ ind ] ) {
      free( block[ ind ] );
      block[ ind ] = NULL;
    } else {
      size_t sz = random() % 10000 + 1;
      block[ ind ] = (char *) malloc( sz );
    }
  }

  // Free anything that's left over.
  for ( int i = 0; i < TEST_SIZE; i++ )
    if ( block[ i ] )
      free( block[ i ] );

  // See what's left, probably one big block.
  reportString( "-------- randomTest --------\n" );
  reportFreeList();
}

int main() {
  babyTest();
  toddlerTest();
  randomTest();
}
