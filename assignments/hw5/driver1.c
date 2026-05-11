// Driver program that's simple enough that we can tell what should
// happen.  Two players have sufficiently similar skill that they
// can play togehter.

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

// A player with skill level 1
void *larryThread( void *arg ) {
  const char *name = "Larry";

  // Try to play after 1 second.
  sleep( 1 );
  
  printf( "%s wants to play\n", name );
  if ( findMatch( name, 1 ) ) {
    printf( "%s is playing\n", name );
    // Play for one second, then exit.
    sleep( 1 );
    donePlaying();
    printf( "%s is done\n", name );
  } else
    printf( "Error: %s didn't find a match\n", name );
  
  return NULL;
}

// A player with skill level 10
void *margeThread( void *arg ) {
  const char *name = "Marge";

  // Try to play after 2 seconds.
  sleep( 2 );
  
  printf( "%s wants to play\n", name );
  if ( findMatch( name, 10 ) ) {
    printf( "%s is playing\n", name );
    
    // Play for one second, then exit.
    sleep( 1 );
    donePlaying();
    printf( "%s is done\n", name );
  } else
    printf( "Error: %s didn't find a match\n", name );

  return NULL;
}

int main( int argc, char *argv[] ) {
  // For testing, to buffer only one output line at a time.
  setvbuf( stdout, NULL, _IOLBF, 0 );
  
  // Initialize our monitor.
  initMonitor( 10, 10 );

  // Make a few threads
  int N = 2;
  pthread_t thread[ N ];

  int i = 0;
  if ( pthread_create( thread + i++, NULL, larryThread, NULL ) != 0 ||
       pthread_create( thread + i++, NULL, margeThread, NULL ) != 0 )
    fail( "Can't create one of the threads.\n" );

  // Threads should be done before time 4.
  sleep( 4 );

  printf( "Stopping matchmaker\n" );
  stopMatchmaking();

  for ( int i = 0; i < N; i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyMonitor();

  return 0;
}
