#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

// Sector size for a 1.44 MB floppy.
#define SSIZE 512

// Start of the (first) copy of the FAT on the disk (Sector 1).
#define FAT_START (1 * SSIZE)

// Marker for the end of a chain of sectors (i.e., no next sector).
#define CHAIN_END 0xFFF
#define BAD_SECTOR 0xFF7

// --- Root Directory Specifics (Standard FAT12 1.44MB) ---
#define NUM_FATS 2
#define SECTORS_PER_FAT 9
#define ROOT_DIR_BYTE_OFFSET ((1 + NUM_FATS * SECTORS_PER_FAT) * SSIZE)
#define ROOT_DIR_SECTORS 14
#define ROOT_DIRECTORY_FLAG -1
#define EOC_MARKER_START 0xFF8
// --- End Root Directory Specifics ---

#define DSIZE 32 // Size of a directory entry

// Byte offset for the start of the data region (after root dir).
#define DATA_START (ROOT_DIR_BYTE_OFFSET + (ROOT_DIR_SECTORS * SSIZE))

// Fail function
static void fail(char const *format, ...)
{
    va_list param;
    va_start(param, format);
    vfprintf(stderr, format, param);
    va_end(param);
    exit(EXIT_FAILURE);
}

// Get FAT12 entry
int getFat12(FILE *fp, int idx)
{
    int offset = idx + (idx / 2);
    unsigned char fatPair[3];
    // Save current position
    long currentPos = ftell(fp);
    if (currentPos == -1)
    {
        perror("ftell before getFat12");
        // Decide how to handle, maybe return an error indicator?
        return CHAIN_END; // Or another error value
    }

    if (fseek(fp, FAT_START + offset, SEEK_SET) != 0)
    {
        fprintf(stderr, "Warning: fseek failed in getFat12 for cluster %d\n", idx);
        // Restore position before returning error
        fseek(fp, currentPos, SEEK_SET);
        return CHAIN_END; // Indicate error
    }
    if (fread(fatPair, 1, 3, fp) != 3)
    {
        // Restore position before failing
        fseek(fp, currentPos, SEEK_SET);
        // Check for EOF vs actual error
        if (feof(fp))
        {
            fprintf(stderr, "Warning: Unexpected EOF reading FAT entry for cluster %d\n", idx);
            return CHAIN_END; // Treat as end of chain on EOF
        }
        else
        {
            fail("Can't read fat entry for cluster: %d at offset %d\n", idx, FAT_START + offset);
        }
    }

    // Restore original file position after reading FAT
    if (fseek(fp, currentPos, SEEK_SET) != 0)
    {
        perror("fseek failed restoring position after getFat12");
        // This is problematic, state is inconsistent. Maybe fail hard?
        fail("Failed to restore file position after FAT read.\n");
    }

    int val1 = fatPair[0];
    int val2 = fatPair[1];
    int val3 = fatPair[2];

    if (idx & 0x1)
    { // Odd
        return ((val2 & 0xF0) >> 4) | (val3 << 4);
    }
    else
    { // Even
        return val1 | ((val2 & 0x0F) << 8);
    }
}

/** Print the contents of a directory. */
void reportDirectory(FILE *fp, int depth, int sec)
{
    unsigned char entry[DSIZE];
    bool end_of_directory = false; // Flag to stop processing sectors if 0x00 found (local to this call)

    if (sec == ROOT_DIRECTORY_FLAG)
    {
        // --- Handle Root Directory ---
        // Iterate through all potential root directory sectors
        for (int sector_index = 0; sector_index < ROOT_DIR_SECTORS; sector_index++)
        {
            long offset = ROOT_DIR_BYTE_OFFSET + ((long)sector_index * SSIZE);
            if (fseek(fp, offset, SEEK_SET) != 0)
            {
                fprintf(stderr, "Warning: Seek failed for root directory sector %d\n", sector_index);
                break; // Cannot continue if seek fails
            }

            // Process entries within this sector
            for (int i = 0; i < SSIZE / DSIZE; i++)
            {
                long currentEntryPos = ftell(fp); // Position before reading entry
                if (fread(entry, 1, DSIZE, fp) != DSIZE)
                {
                    if (feof(fp))
                    { // Reached end of file unexpectedly
                        end_of_directory = true;
                        break;
                    }
                    else
                    { // Actual read error
                        fprintf(stderr, "Warning: Failed to read entry %d in root sector %d. Skipping sector.\n", i, sector_index);
                        end_of_directory = true; // Stop processing this dir due to error
                        break;
                    }
                }

                unsigned char firstByte = entry[0];
                if (firstByte == 0x00)
                { // End of directory marker
                    end_of_directory = true;
                    break; // Stop processing entries IN THIS SECTOR
                }

                // Skip deleted, volume label, LFN
                if (firstByte == 0xE5 || (entry[11] & 0x08) || entry[11] == 0x0F)
                    continue;

                // Skip . and .. (though not usually in root)
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

                for (int d = 0; d < depth; d++)
                    printf("   "); // Use 3 spaces

                if (isDir)
                {
                    printf("%-12s %10s\n", fullName, "DIR");
                    // Ensure file pointer is reset AFTER potential recursive call returns
                    long nextEntryPos = ftell(fp); // Position after reading entry data, before recursion
                    if (startCluster >= 2)
                    {
                        reportDirectory(fp, depth + 1, startCluster);
                        // ***** CRITICAL: Restore file pointer for parent dir *****
                        if (fseek(fp, nextEntryPos, SEEK_SET) != 0)
                        {
                            fail("FATAL: Could not reset file pointer after recursive call for '%s'.\n", fullName);
                        }
                    }
                    else if (startCluster != 0)
                    {
                        fprintf(stderr, "Warning: Directory '%s' points to invalid cluster %d\n", fullName, startCluster);
                    }
                }
                else
                {
                    printf("%-12s %10d\n", fullName, size);
                }
            } // End entry loop for sector

            // After processing entries in a sector, check if the 0x00 marker was found.
            // If so, no need to check subsequent sectors for the root directory.
            if (end_of_directory)
            {
                break; // Stop processing further sectors for the root directory
            }
        } // End sector loop for root directory
    }
    else if (sec >= 2)
    { // Cluster numbers start at 2
        // --- Handle Subdirectory ---
        int currentCluster = sec;

        while (currentCluster < EOC_MARKER_START && currentCluster >= 2 && currentCluster != BAD_SECTOR)
        {
            long offset = DATA_START + ((long)(currentCluster - 2) * SSIZE);
            if (fseek(fp, offset, SEEK_SET) != 0)
            {
                fprintf(stderr, "Warning: Seek failed for subdirectory cluster %d\n", currentCluster);
                break; // Cannot continue this chain
            }

            // Process entries within this cluster's sector
            for (int i = 0; i < SSIZE / DSIZE; i++)
            {
                long currentEntryPos = ftell(fp); // Position before reading entry
                if (fread(entry, 1, DSIZE, fp) != DSIZE)
                {
                    if (feof(fp))
                    {
                        end_of_directory = true;
                        break;
                    }
                    else
                    {
                        fprintf(stderr, "Warning: Failed to read entry %d in cluster %d. Skipping cluster.\n", i, currentCluster);
                        end_of_directory = true; // Stop processing this dir due to error
                        break;
                    }
                }

                unsigned char firstByte = entry[0];
                if (firstByte == 0x00)
                {
                    end_of_directory = true;
                    break; // Stop processing entries in this sector
                }

                if (firstByte == 0xE5 || (entry[11] & 0x08) || entry[11] == 0x0F)
                    continue;

                if (entry[0] == '.') // Skip . and ..
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

                for (int d = 0; d < depth; d++)
                    printf("    ");

                if (isDir)
                {
                    printf("%-12s %10s\n", fullName, "DIR");
                    long nextEntryPos = ftell(fp); // Position after reading entry data, before recursion
                    if (startCluster >= 2)
                    {
                        reportDirectory(fp, depth + 1, startCluster);
                        // ***** CRITICAL: Restore file pointer for parent dir *****
                        if (fseek(fp, nextEntryPos, SEEK_SET) != 0)
                        {
                            fail("FATAL: Could not reset file pointer after recursive call for '%s'.\n", fullName);
                        }
                    }
                    else if (startCluster != 0)
                    {
                        fprintf(stderr, "Warning: Directory '%s' points to invalid cluster %d\n", fullName, startCluster);
                    }
                }
                else
                {
                    printf("%-12s %10d\n", fullName, size);
                }
            } // End entry loop for sector

            if (end_of_directory)
            {
                break; // Stop processing clusters for this directory
            }

            // IMPORTANT: getFat12 now restores the file pointer, so we don't need to seek back here.
            // If getFat12 didn't restore, we'd need: fseek(fp, offset + SSIZE, SEEK_SET); or similar.

            // Get the next cluster from the FAT
            currentCluster = getFat12(fp, currentCluster);

        } // End while loop for subdirectory clusters
    }
    else if (sec != 0 && sec != ROOT_DIRECTORY_FLAG)
    { // Check for invalid non-zero, non-root flag values
        fprintf(stderr, "Warning: Invalid starting sector/cluster %d for directory listing.\n", sec);
    }
} // end reportDirectory

int main(int argc, char *argv[])
{
    if (argc != 2)
        fail("usage: floppyDir <floppy-image>\n");

    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
        fail("Can't open file: %s\n", argv[1]);

    reportDirectory(fp, 0, ROOT_DIRECTORY_FLAG);

    fclose(fp);
    return EXIT_SUCCESS;
}