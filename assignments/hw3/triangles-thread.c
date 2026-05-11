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
#include <sys/syscall.h>
#include <sys/wait.h>
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>

typedef struct
{
    int wid;
    int workers;
    bool report;
    int localCount;
} ThreadInfo;

// Print out an error message and exit.
static void
fail(char const *message)
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

void *findTriangles(void *args)
{
    ThreadInfo *tArgs = (ThreadInfo *)args;
    int wid = tArgs->wid;
    int workers = tArgs->workers;
    bool report = tArgs->report;
    int localCount = 0;
    pid_t tid = syscall(__NR_gettid);

    for (int i = wid; i < vCount - 2; i += workers)
    {
        for (int j = i + 1; j < vCount - 1; j++)
        {
            for (int k = j + 1; k < vCount; k++)
            {
                if (checkTriangle(vList[i], vList[j], vList[k]))
                {
                    localCount++;
                    if (report)
                    {
                        // printf("I’m process %d. Local count: %d. Triangle (%d, %d, %d) found at: %d, %d and %d.\n", pid, localCount, vList[i], vList[j], vList[k], i, j, k);
                        printf("I’m thread %d. Local count: %d. Triangle (%d, %d, %d) found at: %d, %d and %d.\n", tid, localCount, vList[i], vList[j], vList[k], i, j, k);
                    }
                }
            }
        }
    }

    tArgs->localCount = localCount;
    return NULL;
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

    pthread_t threads[workers];
    ThreadInfo tArgs[workers];

    // Create threads
    for (int i = 0; i < workers; i++)
    {
        tArgs[i].wid = i;
        tArgs[i].workers = workers;
        tArgs[i].report = report;
        tArgs[i].localCount = 0;
        if (pthread_create(&threads[i], NULL, findTriangles, &tArgs[i]) != 0)
        {
            fail("Error creating thread");
        }
    }

    // Wait for all threads to finish
    int totalTriangles = 0;
    for (int i = 0; i < workers; i++)
    {
        pthread_join(threads[i], NULL);
        totalTriangles += tArgs[i].localCount;
    }

    printf("Total count: %d\n", totalTriangles);

    return 0;
}
