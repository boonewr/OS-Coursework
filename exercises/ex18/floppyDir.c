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
// #define DATA_START 15872

#define BYTE_OFFSET ((1 + NUM_FATS * SECTORS_PER_FAT) * SSIZE)
#define SECTORS 14

#define BAD_SECTOR 0xFF7
#define EOC_MARKER_START 0xFF8

#define NUM_FATS 2
#define SECTORS_PER_FAT 9

#define DATA_START (BYTE_OFFSET + (SECTORS * SSIZE)) // 16896

// General-purpose fail function with printf-style parameters.
static void fail(char const *format, ...)
{
    va_list param;
    va_start(param, format);
    vprintf(format, param);
    va_end(param);

    exit(EXIT_FAILURE);
}

// Get entry idx in the FAT12.
int getFat12(FILE *fp, int idx)
{
    // Offset for the 3 bytes containing the FAT entry we need.
    int offset = idx / 2 * 3;

    // Read a pair of 12-bit fat entries into the low-order bits of
    // pair.  This assumes the host is little-endian byte order.
    int pair = 0;
    fseek(fp, FAT_START + offset, SEEK_SET);
    if (fread(&pair, 1, 3, fp) != 3)
        fail("Can't read fat entry at: %d\n", idx);

    // Return the right one, based on the low bit of idx.
    if (idx & 0x1)
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
void reportDirectory(FILE *fp, int depth, int sec)
{
    unsigned char entry[DSIZE];
    bool end_of_directory = false;

    // Root
    if (sec < 0)
    {
        for (int sector_index = 0; sector_index < SECTORS; sector_index++)
        {
            long offset = BYTE_OFFSET + ((long)sector_index * SSIZE);
            if (fseek(fp, offset, SEEK_SET) != 0)
            {
                break;
            }

            for (int i = 0; i < SSIZE / DSIZE; i++)
            {
                if (fread(entry, 1, DSIZE, fp) != DSIZE)
                {
                    if (feof(fp))
                    {
                        end_of_directory = true;
                        break;
                    }
                }

                unsigned char firstByte = entry[0];
                if (firstByte == 0x00)
                {
                    end_of_directory = true;
                    break;
                }
                if (firstByte == 0xE5 || (entry[11] & 0x08) || entry[11] == 0x0F)
                    continue;
                if (entry[0] == '.')
                    continue;

                bool isDir = (entry[11] & 0x10) == 0x10;
                char name[9] = {0}, ext[4] = {0};
                strncpy(name, (char *)entry, 8);
                strncpy(ext, (char *)entry + 8, 3);
                for (int j = 7; j >= 0 && name[j] == ' '; j--)
                    name[j] = '\0';
                for (int j = 2; j >= 0 && ext[j] == ' '; j--)
                    ext[j] = '\0';
                char fullName[13];
                if (ext[0])
                    snprintf(fullName, sizeof(fullName), "%s.%s", name, ext);
                else
                    snprintf(fullName, sizeof(fullName), "%s", name);
                int startCluster = entry[26] | (entry[27] << 8);
                int size = entry[28] | (entry[29] << 8) | (entry[30] << 16) | (entry[31] << 24);

                // 4 spaces
                for (int d = 0; d < depth; d++)
                    printf("    ");

                if (isDir)
                {
                    printf("%-12s %10s\n", fullName, "DIR");
                    long nextEntryPos = ftell(fp);
                    if (startCluster >= 2)
                    {
                        reportDirectory(fp, depth + 1, startCluster);
                        if (fseek(fp, nextEntryPos, SEEK_SET) != 0)
                        {
                            fail("Could not reset pointer");
                        }
                    }
                }
                else
                {
                    printf("%-12s %10d\n", fullName, size);
                }
            }
            if (end_of_directory)
                break;
        }
    }
    // Subdirectory
    else if (sec >= 2)
    {
        int currentCluster = sec;
        while (currentCluster < EOC_MARKER_START && currentCluster >= 2 && currentCluster != BAD_SECTOR)
        {
            long offset = DATA_START + ((long)(currentCluster - 2) * SSIZE);
            if (fseek(fp, offset, SEEK_SET) != 0)
            {
                fail("Seek failed");
            }

            for (int i = 0; i < SSIZE / DSIZE; i++)
            {
                if (fread(entry, 1, DSIZE, fp) != DSIZE)
                {
                    if (feof(fp))
                    {
                        end_of_directory = true;
                        break;
                    }
                }

                unsigned char firstByte = entry[0];
                if (firstByte == 0x00)
                {
                    end_of_directory = true;
                    break;
                }
                if (firstByte == 0xE5 || (entry[11] & 0x08) || entry[11] == 0x0F)
                    continue;
                if (entry[0] == '.')
                    continue;

                bool isDir = (entry[11] & 0x10) == 0x10;
                char name[9] = {0}, ext[4] = {0};
                strncpy(name, (char *)entry, 8);
                strncpy(ext, (char *)entry + 8, 3);
                for (int j = 7; j >= 0 && name[j] == ' '; j--)
                    name[j] = '\0';
                for (int j = 2; j >= 0 && ext[j] == ' '; j--)
                    ext[j] = '\0';
                char fullName[13];
                if (ext[0])
                    snprintf(fullName, sizeof(fullName), "%s.%s", name, ext);
                else
                    snprintf(fullName, sizeof(fullName), "%s", name);
                int startCluster = entry[26] | (entry[27] << 8);
                int size = entry[28] | (entry[29] << 8) | (entry[30] << 16) | (entry[31] << 24);

                // 4 spaces
                for (int d = 0; d < depth; d++)
                    printf("    ");

                if (isDir)
                {
                    printf("%-12s %10s\n", fullName, "DIR");
                    long nextEntryPos = ftell(fp);
                    if (startCluster >= 2)
                    {
                        reportDirectory(fp, depth + 1, startCluster);
                        if (fseek(fp, nextEntryPos, SEEK_SET) != 0)
                        {
                            fail("Could not reset pointer");
                        }
                    }
                }
                else
                {
                    printf("%-12s %10d\n", fullName, size);
                }
            }
            if (end_of_directory)
                break;
            currentCluster = getFat12(fp, currentCluster);
        }
    }
}

int main(int argc, char *argv[])
{
    // Check the command-line arguments.
    if (argc != 2)
        fail("usage: floppyDir <floppy-image>\n");

    // Open the disk image file.
    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
        fail("Can't open file: %s\n", argv[1]);

    // Report the directory tree.
    reportDirectory(fp, 0, ROOT_DIR_START);

    // Clean up and exit.
    fclose(fp);
    return EXIT_SUCCESS;
}
