/**
 * @file exclude.c
 * 
 * This program reads in an input text file and a number from command line arguments and outputs
 * the file line by line, excluding the indicated line number.
 */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

/** Maximum number of lines to read from a file */
#define LINES_LIMIT 1000
/** Maximum length of a line in the file */
#define LINE_LIMIT 1000

/**
 * Writes error message to standard error and exits.
 */
void usage()
{
    char *msg = "usage: <input-file> <output-file> <line-number>\n";
    write(2, msg, 48);
    _exit(1);
}

/**
 * Helper function parse an integer from a string.
 * 
 * @param str The string to parse.
 * @return integer representation of the given string.
 */
int string_to_int(const char *str)
{
    int i = 0;
    int out = 0;

    while (str[i] != '\0')
    {
        if (str[i] < '0' || str[i] > '9')
        {
            usage();
        }
        out *= 10;
        out += (str[i] - '0');
        i++;
    }

    return out;
}

/**
 * Main function to read in a file and output its contents minus the indicated line.
 */
int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        usage();
    }

    int inHandle = open(argv[1], O_RDONLY | O_CREAT, 0700);
    int outHandle = open(argv[2], O_RDWR | O_CREAT, 0700);
    if (inHandle < 0 || outHandle < 0)
    {
        usage();
    }

    int excludedLine = string_to_int(argv[3]);

    // Since the first line is considered line 1, 0 and smaller are not valid.
    if (excludedLine <= 0) {
        usage();
    }

    // Since an argument of 1 should exclude the first line of the file
    int excludedLineIdx = excludedLine - 1;

    int len;
    char buffer[64] = {};

    int j = 0;
    int h = 0;
    char wholeFile[LINES_LIMIT][LINE_LIMIT] = {};

    // i => iterating over the read in buffer
    // j => current line number to append to
    // h => position in the current line to add at
    while ((len = read(inHandle, buffer, sizeof(buffer))) > 0)
    {
        for (int i = 0; i < len; i++)
        {
            wholeFile[j][h] = buffer[i];
            h++;
            if (buffer[i] == '\n')
            {
                if (j != excludedLineIdx)
                {
                    write(outHandle, wholeFile[j], h);
                }
                j++;
                h = 0;
            }
        }
    }

    close(inHandle);
    close(outHandle);
    return 0;
}