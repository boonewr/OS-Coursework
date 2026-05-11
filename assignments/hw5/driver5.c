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
  char const *name;  // Name of the person.
  int skill;         // Skill level for this person.
  int playTime;      // How long to play a game.
  int restTime;      // How long to rest between games.
  int gameCount;     // Number of games played by this person.
  pthread_t thread;  // Handle for this person's thread.
} PType;

// Generic implementation of a player.
void *play( void *arg ) {
  // Our argument is really a PType struct.
  PType *ptype = (PType *) arg;

  while ( true ) {
    // Rest for a moment.
    usleep( rand() % ( ptype->restTime * TIME_MULT ) + 1 );

    // Exit when we fail to get an opponent.
    if ( ! findMatch( ptype->name, ptype->skill ) ) {
      return NULL;
    }

    ptype->gameCount += 1;

    // Try to stop after a random amount of time
    usleep( rand() % ( ptype->playTime * TIME_MULT ) + 1 );
    
    donePlaying();
  }
  
  return NULL;
}

int main( int argc, char *argv[] ) {
  // For testing, to buffer only one output line at a time.
  setvbuf( stdout, NULL, _IOLBF, 0 );

  // Behavior of each player, ad different skill levels and
  // wanting to play for different amounts of time.
  PType plist[] = {
    { "Adelaida", 0, 8, 120 },
    { "Eloisa", 0, 12, 140 },
    { "Scott", 0, 16, 160 },
    { "Benny", 1, 20, 180 },
    { "Kylie", 2, 8, 120 },
    { "Leisa", 3, 12, 140 },
    { "Jarred", 8, 16, 160 },
    { "Gino", 9, 20, 180 },
    { "Delfina", 8, 10, 120 },
    { "Elsa", 10, 12, 140 },
    { "Willie", 10, 16, 160 },
    { "Tena", 10, 20, 180 },
    { "Brett", 11, 8, 120 },
    { "Luella", 12, 12, 140 },
    { "Tiana", 17, 16, 160 },
    { "Marquita", 18, 20, 180 },
    { "Caitlyn", 19, 8, 120 },
    { "Orpha", 20, 12, 140 },
    { "Archie", 20, 16, 160 },
    { "Kurtis", 20, 20, 180 }
  };
  
  // Initialize our monitor.
  initMonitor( 20, 8 );

  // Start threads for each player.
  int N = sizeof( plist ) / sizeof( plist[ 0 ] );;
  for ( int i = 0; i < N; i++ ) {
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
