#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

/** A little record used to keep up with each of our threads. */
typedef struct {
  // Thread handle for this chef.
  pthread_t thread;

  // Name for this chef.
  char name[ 25 ];

  // Poitner to the start routine for this thread.
  void *(*start)( void * );

  // Number of dishes prepared by this chef.
  int dishCount;
} ChefRec;

/** To tell all the chefs when they can quit running. */
static bool running = true;

/** Print out an error message and exit. */
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}
/** Called by a chef after they have locked all the required appliances
    and are ready to cook for about the given number of milliseconds. */
static void cook( int duration, ChefRec *chef )
{
  printf( "%s is cooking\n", chef->name );
  // Wait for between duration / 2 and duration ms
  usleep( 500 * ( (long) rand() * duration / RAND_MAX + duration  ) );
  chef->dishCount++;
}

/** Called by a chef between dishes, to let them rest about the given
    number of milliseconds before cooking another dish. */
static void rest( int duration, ChefRec *chef )
{
  printf( "%s is resting\n", chef->name );
  // Wait for between duration / 2 and duration ms
  usleep( 500 * ( (long) rand() * duration / RAND_MAX + duration  ) );
}

// A mutex representing the lock on each appliance.
// Acquiring the needed mutexes before cooking prevents two
// chefs from trying to use the same appliance at the same time.
static pthread_mutex_t mixer = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t blender = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t oven = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t grill = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t fryer = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t griddle = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t coffeeMaker = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t microwave = PTHREAD_MUTEX_INITIALIZER;

/** Tad is a chef needing 60 milliseconds to prepare a dish. */
void *Tad( void * arg )
{
  // Argument struct, converted to its actual type.
  ChefRec *rec = (ChefRec *) arg;

  while ( running ) {
    // Get the appliances this chef uses.
    pthread_mutex_lock( &griddle );
    pthread_mutex_lock( &grill );
    pthread_mutex_lock( &microwave );

    cook( 60, rec );

    pthread_mutex_unlock( &microwave );
    pthread_mutex_unlock( &grill );
    pthread_mutex_unlock( &griddle );

    rest( 25, rec );
  }

  return NULL;}

/** Merry is a chef needing 15 milliseconds to prepare a dish. */
void *Merry( void * arg )
{
  // Argument struct, converted to its actual type.
  ChefRec *rec = (ChefRec *) arg;

  while ( running ) {
    // Get the appliances this chef uses.
    pthread_mutex_lock( &mixer );
    pthread_mutex_lock( &oven );
    pthread_mutex_lock( &blender );

    cook( 15, rec );

    pthread_mutex_unlock( &blender );
    pthread_mutex_unlock( &oven );
    pthread_mutex_unlock( &mixer );

    rest( 25, rec );
  }

  return NULL;}

/** Charles is a chef needing 90 milliseconds to prepare a dish. */
void *Charles( void * arg )
{
  // Argument struct, converted to its actual type.
  ChefRec *rec = (ChefRec *) arg;

  while ( running ) {
    // Get the appliances this chef uses.
    pthread_mutex_lock( &coffeeMaker );
    pthread_mutex_lock( &griddle );

    cook( 90, rec );

    pthread_mutex_unlock( &griddle );
    pthread_mutex_unlock( &coffeeMaker );

    rest( 25, rec );
  }

  return NULL;}

/** Merlin is a chef needing 15 milliseconds to prepare a dish. */
void *Merlin( void * arg )
{
  // Argument struct, converted to its actual type.
  ChefRec *rec = (ChefRec *) arg;

  while ( running ) {
    // Get the appliances this chef uses.
    pthread_mutex_lock( &blender );
    pthread_mutex_lock( &mixer );

    cook( 15, rec );

    pthread_mutex_unlock( &mixer );
    pthread_mutex_unlock( &blender );

    rest( 25, rec );
  }

  return NULL;}

/** Lyn is a chef needing 75 milliseconds to prepare a dish. */
void *Lyn( void * arg )
{
  // Argument struct, converted to its actual type.
  ChefRec *rec = (ChefRec *) arg;

  while ( running ) {
    // Get the appliances this chef uses.
    pthread_mutex_lock( &griddle );
    pthread_mutex_lock( &fryer );

    cook( 75, rec );

    pthread_mutex_unlock( &fryer );
    pthread_mutex_unlock( &griddle );

    rest( 25, rec );
  }

  return NULL;}

/** Marian is a chef needing 45 milliseconds to prepare a dish. */
void *Marian( void * arg )
{
  // Argument struct, converted to its actual type.
  ChefRec *rec = (ChefRec *) arg;

  while ( running ) {
    // Get the appliances this chef uses.
    pthread_mutex_lock( &coffeeMaker );
    pthread_mutex_lock( &microwave );
    pthread_mutex_lock( &oven );

    cook( 45, rec );

    pthread_mutex_unlock( &oven );
    pthread_mutex_unlock( &microwave );
    pthread_mutex_unlock( &coffeeMaker );

    rest( 25, rec );
  }

  return NULL;}

/** Summer is a chef needing 30 milliseconds to prepare a dish. */
void *Summer( void * arg )
{
  // Argument struct, converted to its actual type.
  ChefRec *rec = (ChefRec *) arg;

  while ( running ) {
    // Get the appliances this chef uses.
    pthread_mutex_lock( &fryer );
    pthread_mutex_lock( &blender );
    pthread_mutex_lock( &coffeeMaker );

    cook( 30, rec );

    pthread_mutex_unlock( &coffeeMaker );
    pthread_mutex_unlock( &blender );
    pthread_mutex_unlock( &fryer );

    rest( 25, rec );
  }

  return NULL;}

/** Sammy is a chef needing 60 milliseconds to prepare a dish. */
void *Sammy( void * arg )
{
  // Argument struct, converted to its actual type.
  ChefRec *rec = (ChefRec *) arg;

  while ( running ) {
    // Get the appliances this chef uses.
    pthread_mutex_lock( &grill );
    pthread_mutex_lock( &fryer );

    cook( 60, rec );

    pthread_mutex_unlock( &fryer );
    pthread_mutex_unlock( &grill );

    rest( 25, rec );
  }

  return NULL;}

/** Lura is a chef needing 15 milliseconds to prepare a dish. */
void *Lura( void * arg )
{
  // Argument struct, converted to its actual type.
  ChefRec *rec = (ChefRec *) arg;

  while ( running ) {
    // Get the appliances this chef uses.
    pthread_mutex_lock( &microwave );
    pthread_mutex_lock( &mixer );

    cook( 15, rec );

    pthread_mutex_unlock( &mixer );
    pthread_mutex_unlock( &microwave );

    rest( 25, rec );
  }

  return NULL;}

/** Ginny is a chef needing 45 milliseconds to prepare a dish. */
void *Ginny( void * arg )
{
  // Argument struct, converted to its actual type.
  ChefRec *rec = (ChefRec *) arg;

  while ( running ) {
    // Get the appliances this chef uses.
    pthread_mutex_lock( &grill );
    pthread_mutex_lock( &oven );

    cook( 45, rec );

    pthread_mutex_unlock( &oven );
    pthread_mutex_unlock( &grill );

    rest( 25, rec );
  }

  return NULL;}

int main( void )
{
  // Seed the random number generator, so we get variation in behavior.
  srand( time( NULL ) );

  // Make a record for each chef.
  ChefRec chefList[] = {
    { .name = "Tad", .start = Tad },
    { .name = "Merry", .start = Merry },
    { .name = "Charles", .start = Charles },
    { .name = "Merlin", .start = Merlin },
    { .name = "Lyn", .start = Lyn },
    { .name = "Marian", .start = Marian },
    { .name = "Summer", .start = Summer },
    { .name = "Sammy", .start = Sammy },
    { .name = "Lura", .start = Lura },
    { .name = "Ginny", .start = Ginny },
  };

  // Make a thread for each chef.
  for ( int i = 0; i < sizeof(chefList) / sizeof(chefList[0]); i++ ) {
    // Give each chef a pointer to its ChefRec struct.
    if ( pthread_create( &chefList[ i ].thread,
                         NULL,
                         chefList[ i ].start,
                         chefList + i ) != 0 )
      fail( "Can't create thread" );
  }

  // Let the chefs cook for a while, then ask them to stop.
  sleep( 10 );
  running = false;

  // Wait for all our chefs to finish, and collect up how much
  // cooking was done.
  int total = 0;
  for ( int i = 0; i < sizeof(chefList) / sizeof(chefList[0]); i++ ) {
    pthread_join( chefList[ i ].thread, NULL );
    printf( "%s cooked %d dishes\n",
            chefList[ i ].name,
            chefList[ i ].dishCount );
    total += chefList[ i ].dishCount;
  }
  printf( "Total dishes cooked: %d\n", total );
}
