#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

// Print an error message and exit on an error
void fail( char *msg ) {
  fprintf( stderr, "Error: %s\n", msg );
  exit( 1 );
}

// Global variable, to tell all the threads when it's time to stop.
volatile int running = 1;

// A semaphore for each object someone might need to use.
sem_t hammer;
sem_t rubber_stamp;
sem_t bookmark;
sem_t shovel;
sem_t bowtie;
sem_t camera;
sem_t spatula;
sem_t tricorder;
sem_t usb_cable;

// Start routine for thread Tamara.
void *threadTamara( void *arg ) {
  while ( running ) {
    // Acquire locks on the objects I'm going to use.
    sem_wait( &bookmark );
    sem_wait( &shovel );
    sem_wait( &tricorder );

    // Say my name.
    printf( "Tamara\n" );

    // Release locks on the objects I'm used.
    sem_post( &bookmark );
    sem_post( &shovel );
    sem_post( &tricorder );
  }
  return NULL;
}

// Start routine for thread Adolfo.
void *threadAdolfo( void *arg ) {
  while ( running ) {
    // Acquire locks on the objects I'm going to use.
    sem_wait( &hammer );
    sem_wait( &bowtie );
    sem_wait( &camera );
    sem_wait( &usb_cable );

    // Say my name.
    printf( "Adolfo\n" );

    // Release locks on the objects I'm used.
    sem_post( &hammer );
    sem_post( &bowtie );
    sem_post( &camera );
    sem_post( &usb_cable );
  }
  return NULL;
}

// Start routine for thread Terrie.
void *threadTerrie( void *arg ) {
  while ( running ) {
    // Acquire locks on the objects I'm going to use.
    sem_wait( &hammer );
    sem_wait( &rubber_stamp );
    sem_wait( &bookmark );
    sem_wait( &bowtie );

    // Say my name.
    printf( "Terrie\n" );

    // Release locks on the objects I'm used.
    sem_post( &hammer );
    sem_post( &rubber_stamp );
    sem_post( &bookmark );
    sem_post( &bowtie );
  }
  return NULL;
}

// Start routine for thread Dollie.
void *threadDollie( void *arg ) {
  while ( running ) {
    // Acquire locks on the objects I'm going to use.
    sem_wait( &hammer );
    sem_wait( &shovel );

    // Say my name.
    printf( "Dollie\n" );

    // Release locks on the objects I'm used.
    sem_post( &hammer );
    sem_post( &shovel );
  }
  return NULL;
}

// Start routine for thread Alesha.
void *threadAlesha( void *arg ) {
  while ( running ) {
    // Acquire locks on the objects I'm going to use.
    sem_wait( &shovel );
    sem_wait( &camera );
    sem_wait( &usb_cable );

    // Say my name.
    printf( "Alesha\n" );

    // Release locks on the objects I'm used.
    sem_post( &shovel );
    sem_post( &camera );
    sem_post( &usb_cable );
  }
  return NULL;
}

// Start routine for thread Benita.
void *threadBenita( void *arg ) {
  while ( running ) {
    // Acquire locks on the objects I'm going to use.
    sem_wait( &hammer );
    sem_wait( &spatula );

    // Say my name.
    printf( "Benita\n" );

    // Release locks on the objects I'm used.
    sem_post( &hammer );
    sem_post( &spatula );
  }
  return NULL;
}

// Start routine for thread Laverne.
void *threadLaverne( void *arg ) {
  while ( running ) {
    // Acquire locks on the objects I'm going to use.
    sem_wait( &camera );
    sem_wait( &tricorder );
    sem_wait( &usb_cable );

    // Say my name.
    printf( "Laverne\n" );

    // Release locks on the objects I'm used.
    sem_post( &camera );
    sem_post( &tricorder );
    sem_post( &usb_cable );
  }
  return NULL;
}

// Start routine for thread Grady.
void *threadGrady( void *arg ) {
  while ( running ) {
    // Acquire locks on the objects I'm going to use.
    sem_wait( &hammer );
    sem_wait( &usb_cable );

    // Say my name.
    printf( "Grady\n" );

    // Release locks on the objects I'm used.
    sem_post( &hammer );
    sem_post( &usb_cable );
  }
  return NULL;
}

// Start routine for thread Arnold.
void *threadArnold( void *arg ) {
  while ( running ) {
    // Acquire locks on the objects I'm going to use.
    sem_wait( &rubber_stamp );
    sem_wait( &camera );
    sem_wait( &spatula );

    // Say my name.
    printf( "Arnold\n" );

    // Release locks on the objects I'm used.
    sem_post( &rubber_stamp );
    sem_post( &camera );
    sem_post( &spatula );
  }
  return NULL;
}

// Start routine for thread Gladys.
void *threadGladys( void *arg ) {
  while ( running ) {
    // Acquire locks on the objects I'm going to use.
    sem_wait( &camera );
    sem_wait( &spatula );

    // Say my name.
    printf( "Gladys\n" );

    // Release locks on the objects I'm used.
    sem_post( &camera );
    sem_post( &spatula );
  }
  return NULL;
}

// Main program, make all the threads, let them run for a while,
// then tell them to stop.
int main( int argc, char *argv[] ) {
  // initialize the semaphores so only one thread can lock them.
  sem_init( &hammer, 0, 1 );
  sem_init( &rubber_stamp, 0, 1 );
  sem_init( &bookmark, 0, 1 );
  sem_init( &shovel, 0, 1 );
  sem_init( &bowtie, 0, 1 );
  sem_init( &camera, 0, 1 );
  sem_init( &spatula, 0, 1 );
  sem_init( &tricorder, 0, 1 );
  sem_init( &usb_cable, 0, 1 );

  // Handle for each thread.
  pthread_t thread[ 10 ];

  if ( pthread_create( thread + 0, NULL, threadTamara, NULL ) != 0 )
    fail( "Can't create Tamara thread.\n" );
  if ( pthread_create( thread + 1, NULL, threadAdolfo, NULL ) != 0 )
    fail( "Can't create Adolfo thread.\n" );
  if ( pthread_create( thread + 2, NULL, threadTerrie, NULL ) != 0 )
    fail( "Can't create Terrie thread.\n" );
  if ( pthread_create( thread + 3, NULL, threadDollie, NULL ) != 0 )
    fail( "Can't create Dollie thread.\n" );
  if ( pthread_create( thread + 4, NULL, threadAlesha, NULL ) != 0 )
    fail( "Can't create Alesha thread.\n" );
  if ( pthread_create( thread + 5, NULL, threadBenita, NULL ) != 0 )
    fail( "Can't create Benita thread.\n" );
  if ( pthread_create( thread + 6, NULL, threadLaverne, NULL ) != 0 )
    fail( "Can't create Laverne thread.\n" );
  if ( pthread_create( thread + 7, NULL, threadGrady, NULL ) != 0 )
    fail( "Can't create Grady thread.\n" );
  if ( pthread_create( thread + 8, NULL, threadArnold, NULL ) != 0 )
    fail( "Can't create Arnold thread.\n" );
  if ( pthread_create( thread + 9, NULL, threadGladys, NULL ) != 0 )
    fail( "Can't create Gladys thread.\n" );

  // Let the threads run for a few seconds.
  sleep( 5 );
  running = 0;

  for ( int i = 0; i < 10; i++ )
    pthread_join( thread[ i ], NULL );

  return EXIT_SUCCESS;
}
