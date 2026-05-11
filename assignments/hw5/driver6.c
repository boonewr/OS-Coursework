// Larger test driver program, with 20 players of different various levels and
// who like to play at different rates.

#include <stdio.h>
#include <pthread.h>   // for pthreads
#include <stdlib.h>    // for exit
#include <unistd.h>    // for sleep/usleep

#include "matchmaker.h"

// Global multiplier for the time to play or wait between games.
#define TIME_MULT 1000

// General-purpose fail function.  Print a message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Generic definition for a player's behavior.
typedef struct {
  char name[ 30 ];   // Name of the person.
  int skill;         // Skill level for this person.
  int gameCount;     // Number of games played by this person.
  pthread_t thread;  // Handle for this person's thread.
} PType;

// Generic implementation of a player.
void *play( void *arg ) {
  // Our argument is really a PType struct.
  PType *ptype = (PType *) arg;

  while ( true ) {
    // Rest for a moment.
    usleep( rand() % ( TIME_MULT * 100 ) + 1 );

    // Exit when we fail to get an opponent.
    if ( ! findMatch( ptype->name, ptype->skill ) ) {
      return NULL;
    }

    ptype->gameCount += 1;

    // Try to stop after a random amount of time
    usleep( rand() % ( TIME_MULT ) + 1 );
    
    donePlaying();
  }
  
  return NULL;
}

int main( int argc, char *argv[] ) {
  // For testing, to buffer only one output line at a time.
  setvbuf( stdout, NULL, _IOLBF, 0 );

  int N = 102;
  PType plist[ N ];
  
  // Initialize our monitor.
  initMonitor( 50, 10 );

  // Start threads for each player.
  for ( int i = 0; i < N; i++ ) {
    plist[ i ].gameCount = 0;
    plist[ i ].skill = i / 2;
    snprintf( plist[ i ].name, sizeof( plist[ i ].name ), "player-%03d", i );
    if ( pthread_create( & plist[ i ].thread, NULL, play, plist + i ) != 0 )
      fail( "Can't create one of the threads.\n" );
  }

  // Close the matchmaker after 10 seconds.  This should eventually make all
  // the threads exit.
  sleep( 10 );
  stopMatchmaking();

  // Wait for all the threads to finish.
  for ( int i = 0; i < N; i++ )
    pthread_join( plist[ i ].thread, NULL );

  // Report how many times each person got to play and a global total.
  int total = 0;
  for ( int i = 0; i < N; i++ ) {
    printf( "%s played %d games\n", plist[ i ].name, plist[ i ].gameCount );
    total += plist[ i ].gameCount;
  }
  printf( "Total: %d\n", total );

  // Free any monitor resources.
  destroyMonitor();

  return 0;
}
