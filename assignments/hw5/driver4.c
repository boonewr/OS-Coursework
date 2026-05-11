// Driver program that's simple enough that we can tell what should
// happen.  The first two players are too different in skill to play togehter,
// but the next two can each play against one of them.

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

// A player with skill level 5
void *kylieThread( void *arg ) {
  const char *name = "Kylie";

  // Try to play after 1 second.
  sleep( 1 );
  
  printf( "%s wants to play\n", name );
  if ( findMatch( name, 5 ) ) {
    printf( "%s is playing\n", name );
  
    // Play for two seconds, then exit.
    sleep( 2 );
    donePlaying();
    printf( "%s is done\n", name );
  } else
    printf( "Error: %s didn't find a match\n", name );
  
  return NULL;
}

// A player with skill level 5
void *vernonThread( void *arg ) {
  const char *name = "Vernon";

  // Try to play after 2 seconds.
  sleep( 2 );
  
  printf( "%s wants to play\n", name );
  if ( findMatch( name, 5 ) ) {
    printf( "%s is playing\n", name );
  
    // Play for two seconds, then exit.
    sleep( 2 );
    donePlaying();
    printf( "%s is done\n", name );
  } else
    printf( "Error: %s didn't find a match\n", name );
  
  return NULL;
}

// A player with skill level 0
void *ursulaThread( void *arg ) {
  const char *name = "Ursula";

  // Try to play after 3 seconds.
  sleep( 3 );
  
  printf( "%s wants to play\n", name );
  if ( findMatch( name, 0 ) ) {
    printf( "%s is playing\n", name );
  
    // Play for one second, but we'll have to wait for the other player
    // before we can exit.
    sleep( 1 );
    donePlaying();
    printf( "%s is done\n", name );
  } else
    printf( "Error: %s didn't find a match\n", name );
  
  return NULL;
}

// A player with skill level 10
void *lillyThread( void *arg ) {
  const char *name = "Lilly";

  // Try to play after 4 seconds.
  sleep( 4 );
  
  printf( "%s wants to play\n", name );
  if ( findMatch( name, 10 ) ) {
    printf( "%s is playing\n", name );
  
    // Play for one second, but we'll have to wait for the other player
    // before we can exit.
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
  initMonitor( 10, 9 );

  // Make a few threads
  int N = 4;
  pthread_t thread[ N ];

  int i = 0;
  if ( pthread_create( thread + i++, NULL, kylieThread, NULL ) != 0 ||
       pthread_create( thread + i++, NULL, vernonThread, NULL ) != 0 ||
       pthread_create( thread + i++, NULL, ursulaThread, NULL ) != 0 ||
       pthread_create( thread + i++, NULL, lillyThread, NULL ) != 0 )
    fail( "Can't create one of the threads.\n" );

  // All threads should have opponents by time 8
  sleep( 8 );
  
  printf( "Stopping matchmaker\n" );
  stopMatchmaking();

  for ( int i = 0; i < N; i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyMonitor();

  return 0;
}
