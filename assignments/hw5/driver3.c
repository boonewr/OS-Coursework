// Driver program that's simple enough that we can tell what should
// happen.  Three players with skill levels that are too far apart.  They
// wait until the matchmaker closes.

#include <stdio.h>
#include <pthread.h>   // for pthreads
#include <stdlib.h>    // for exit
#include <unistd.h>    // for sleep/usleep

#include "matchmaker.h"

// General-purpose fail function.  Print a message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// A player with skill level 10
void *dionThread( void *arg ) {
  const char *name = "Dion";

  // Try to play after 1 second.
  sleep( 1 );
  
  printf( "%s wants to play\n", name );
  if ( findMatch( name, 10 ) )
    printf( "Error: %s should not find a match\n", name );
  else
    printf( "%s didn't find a match\n", name );
  
  return NULL;
}

// A player with skill level 0
void *fernThread( void *arg ) {
  const char *name = "Fern";

  // Try to play after 2 seconds.
  sleep( 2 );
  
  printf( "%s wants to play\n", name );
  if ( findMatch( name, 0 ) )
    printf( "Error: %s should not find a mtch\n", name );
  else
    printf( "%s didn't find a match\n", name );
  
  return NULL;
}

// A player with skill level 5
void *arlenThread( void *arg ) {
  const char *name = "Arlen";

  // Try to play after 3 seconds.
  sleep( 3 );
  
  printf( "%s wants to play\n", name );
  if ( findMatch( name, 5 ) )
    printf( "Error: %s should not find a match\n", name );
  else
    printf( "%s didn't find a match\n", name );
  
  return NULL;
}

int main( int argc, char *argv[] ) {
  // For testing, to buffer only one output line at a time.
  setvbuf( stdout, NULL, _IOLBF, 0 );
  
  // Initialize our monitor.
  initMonitor( 10, 4 );

  // Make a few threads
  int N = 3;
  pthread_t thread[ N ];

  int i = 0;
  if ( pthread_create( thread + i++, NULL, dionThread, NULL ) != 0 ||
       pthread_create( thread + i++, NULL, fernThread, NULL ) != 0 ||
       pthread_create( thread + i++, NULL, arlenThread, NULL ) != 0 )
    fail( "Can't create one of the threads.\n" );

  // None of the threads should have an opponent when we stop.
  sleep( 4 );
  
  printf( "Stopping matchmaker\n" );
  stopMatchmaking();

  for ( int i = 0; i < N; i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyMonitor();

  return 0;
}
