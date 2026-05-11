// Open an existing message queue, read a message from the user and
// send it over the queue.  Start this program after mqReceiver.

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>

// Maximum message size for communicating over the queue.
#define MAX_SIZE 1024

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

int main() {
  // Open the message queue for talking to the receiver.
  mqd_t myQueue = mq_open( "/sample-queue", O_WRONLY );
  if ( myQueue == -1 )
    fail( "Can't open message queue" );

  // Read a line of text from the user
  char buffer[ MAX_SIZE + 1 ];
  printf( "Enter a message: " );
  fgets( buffer, MAX_SIZE + 1, stdin );

  // Send the message to the receiver.
  mq_send( myQueue, buffer, strlen( buffer ), 0 );

  // We're done, close our copy of the queue (which would happen when we exit anyway).
  mq_close( myQueue );
}
