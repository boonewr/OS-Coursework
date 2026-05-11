#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

// I've hard-coded some of the paramters below.  For a better soluiton,
// these should be obtained from the BIOS parameter block, in the first
// sector of the disk.

// Sector size for a 1.44 MB floppy.
#define SSIZE 512

// Number of sectors on a 1.44 MB floppy.
#define SCOUNT 2880

// Start of the (first) copy of the FAT on the disk.
#define FAT_START 512

// Marker for the end of a chain of sectors (i.e., no next sector).
#define CHAIN_END 4095

// Sector number where the root directory starts.  It's negative because
// the root directory is before the data region on the disk.
#define ROOT_DIR_START -12

// Number of bytes in a directory entry.
#define DSIZE 32

// Byte offset for the start of the data region on the disk.
#define DATA_START 15872

// General-purpose fail function with printf-style parameters.
static void fail( char const *format, ... ) {
  va_list param;
  va_start( param, format );
  vprintf( format, param );
  va_end( param );

  exit( EXIT_FAILURE );
}

// Get entry idx in the FAT12.
int getFat12( FILE *fp, int idx ) {
  // Offset for the 3 bytes containing the FAT entry we need.
  int offset = idx / 2 * 3;

  // Read a pair of 12-bit fat entries into the low-order bits of
  // pair.  This assumes the host is little-endian byte order.
  int pair = 0;
  fseek( fp, FAT_START + offset, SEEK_SET );
  if ( fread( &pair, 1, 3, fp ) != 3 )
    fail( "Can't read fat entry at: %d\n", idx );

  // Return the right one, based on the low bit of idx.
  if ( idx & 0x1 )
    return pair >> 12;
  else
    return pair & 0xFFF;
}

/** Print the contents of a directory (starting) at the given
    sector number.  If sec is negative, then this is the root
    directory and the sectors of the directory are contiguous and
    they're stored before the data region.  If sec is non-negative
    then it's a subdirectory stored in the data region, and the FAT
    determines the order of the sectors.  Depth is how deep the
    (sub)directory is in the file tree, for indentaiton.  fp is the
    pointer to the image file.
*/
void reportDirectory( FILE *fp, int depth, int sec ) {
  // ...
}

int main( int argc, char *argv[] ) {
  // Check the command-line arguments.
  if ( argc != 2 )
    fail( "usage: floppyDir <floppy-image>\n" );
  
  // Open the disk image file.
  FILE *fp = fopen( argv[ 1 ], "rb" );
  if ( ! fp )
    fail( "Can't open file: %s\n", argv[ 1 ] );

  // Report the directory tree.
  reportDirectory( fp, 0, ROOT_DIR_START );

  // Clean up and exit.
  fclose( fp );
  return EXIT_SUCCESS;
}
