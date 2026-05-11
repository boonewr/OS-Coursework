// Create a message queue and wait to get a message from it.
// Run this program first.

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

// Maximum message size for communicating over the queue.
#define MAX_SIZE 1024

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

int main() {
  // Prepare a structure specifying attributes for the message queue
  // we're about to make.
  struct mq_attr attr;
  attr.mq_flags = 0;          // No special flags.
  attr.mq_maxmsg = 10;        // 10 messages of buffer capacity.
  attr.mq_msgsize = MAX_SIZE; // Maximum size of each message.

  // Make a new message queue for reading (based on the attribute structure)
  mqd_t myQueue = mq_open( "/sample-queue", O_RDONLY | O_CREAT, 0600, &attr );
  if ( myQueue == -1 )
    fail( "Can't create request queue" );

  // Buffer for reading a message from the sender, this must be at least as large as the
  // size bound on the queue.
  char buffer[ MAX_SIZE ];
  
  // Try to get a message (this will wait until one arrives).
  int len = mq_receive( myQueue, buffer, sizeof( buffer ), NULL );

  if ( len >= 0 ) {
    printf( "Got message: " );
    // The message isn't null terminated (we don't send the null in this example)
    // We could add a null terminator; instead, we just print it a char at a time.
    for ( int i = 0; i < len; i++ )
      printf( "%c", buffer[ i ] );
  } else
    fail( "Unable to receive message." );

  // Close and delete the message queue.
  mq_close( myQueue );
  mq_unlink( "/sample-queue" ); // Really, we should use a constant here.

  return 0;
}
