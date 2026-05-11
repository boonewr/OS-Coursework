/**
 * @file triangles.c
 * @author Robbie Boone (wrboone)
 *
 * Reads a text file of integers and finds all possible triangles formed by these integers.
 * Takes a number of workers to use for parallelizing triangle checking from command line arguments.
 */

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <stdbool.h>

// Print out an error message and exit.
static void fail(char const *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

// Print out a usage message, then exit.
static void usage()
{
    printf("usage: triangles <workers>\n");
    printf("       triangles <workers> report\n");
    exit(1);
}

// Input sequence of positive integers.
int *vList;

// Number of integers on the list.
int vCount = 0;

// Capacity of the list of integers.
int vCap = 0;

// Read the list of integers.
void readList()
{
    // Set up initial list and capacity.
    vCap = 5;
    vList = (int *)malloc(vCap * sizeof(int));

    // Keep reading as many values as we can.
    int v;
    while (scanf("%d\n", &v) == 1)
    {
        // Grow the list if needed.
        if (vCount >= vCap)
        {
            vCap *= 2;
            vList = (int *)realloc(vList, vCap * sizeof(int));
        }

        // Store the latest value in the next array slot.
        vList[vCount++] = v;
    }
}

/**
 * Checks if a triangle can be formed with the given side lengths
 */
bool checkTriangle(int a, int b, int c)
{
    return (a + b > c) && (a + c > b) && (b + c > a);
}

/**
 * Helper function to find all triangles. Represents one worker.
 */
void findTriangles(int wid, int workers, int pfd, bool report)
{
    int count = 0;
    pid_t pid = getpid();
    for (int i = wid; i < vCount - 2; i += workers)
    {
        for (int j = i + 1; j < vCount - 1; j++)
        {
            for (int k = j + 1; k < vCount; k++)
            {
                if (checkTriangle(vList[i], vList[j], vList[k]))
                {
                    count++;
                    if (report)
                    {
                        /*
                        I’m process 98129. Local count: 4. Triangle (6, 7, 9) found at: 0, 2 and 4.
                        */
                        printf("I’m process %d. Local count: %d. Triangle (%d, %d, %d) found at: %d, %d and %d.\n", pid, count, vList[i], vList[j], vList[k], i, j, k);
                    }
                }
            }
        }
    }

    // Lock pipe, write the count, then unlock pipe for other workers
    lockf(pfd, F_LOCK, 0);
    write(pfd, &count, sizeof(int));
    lockf(pfd, F_ULOCK, 0);
}

int main(int argc, char *argv[])
{
    bool report = false;
    int workers = 4;

    // Parse command-line arguments.
    if (argc < 2 || argc > 3)
        usage();

    if (sscanf(argv[1], "%d", &workers) != 1 ||
        workers < 1)
        usage();

    // If there's a second argument, it better be the word, report
    if (argc == 3)
    {
        if (strcmp(argv[2], "report") != 0)
            usage();
        report = true;
    }

    readList();

    // Create a pipe for communication between parent and workers
    int pfd[2];
    if (pipe(pfd) == -1)
    {
        fail("Error creating pipe");
    }

    // Fork workers
    for (int i = 0; i < workers; i++)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            fail("Error forking");
        }
        else if (pid == 0)
        {
            // Child process i
            close(pfd[0]);
            findTriangles(i, workers, pfd[1], report);
            close(pfd[1]);
            exit(0);
        }
    }

    // Parent
    close(pfd[1]);

    int totalTriangles = 0;
    for (int i = 0; i < workers; i++)
    {
        int count;
        read(pfd[0], &count, sizeof(int));
        totalTriangles += count;
    }

    close(pfd[0]);

    // Wait for all workers
    for (int i = 0; i < workers; i++)
    {
        wait(NULL);
    }

    printf("Total count: %d\n", totalTriangles);

    return 0;
}
