#include <stdio.h>
#include <stdlib.h>

// Recursie quicksort, partition the list using the first value as a pivot,
// then sort the two sides of the partition.
void sort( int *seq, int n ) {
  // A trivial list is already sorted.
  if ( n <= 1 )
    return;
  
  // Partition operation.  The first value is the pivot.
  int pivot = seq[ 0 ];
  // Last value less than the pivot.
  int lower = 0;
  for ( int i = 1; i < n; i++ )
    if ( seq[ i ] < pivot ) {
      int temp = seq[ ++lower ];
      seq[ lower ] = seq[ i ];
      seq[ i ] = temp;
    }

  // Move the pivot to the middle.
  seq[ 0 ] = seq[ lower ];
  seq[ lower ] = pivot;

  // Sort the two halves of the list.
  sort( seq, lower );
  sort( seq + lower + 1, n - lower - 1 );
}

int main( int argc, char *argv[] )
{
  // Parse command-line arguments.
  int n = 10;
  if ( argc >= 2 )
    if ( argc > 2 || sscanf( argv[ 1 ], "%d", &n ) != 1 ||
         n < 0 ) {
      fprintf( stderr, "usage: array <n>\n" );
      exit( EXIT_FAILURE );
    }

  // Make a list of random values.
  int *list = (int *) malloc( sizeof( int ) * n );
  for ( int i = 0; i < n; i++ )
    list[ i ] = rand();

  // Sort them
  sort( list, n );

  // Print the sorted items.
  for ( int i = 0; i < n; i++ )
    printf( "%d\n", list[ i ] );

  // Free memory for the list.
  free( list );

  return 0;
}
