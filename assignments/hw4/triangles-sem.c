#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <semaphore.h>
#include <sys/syscall.h>

// Print out an error message and exit.
static void fail(char const *message)
{
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}

// Print out a usage message, then exit.
static void usage()
{
    printf("usage: triangles-sem <workers>\n");
    printf("       triangles-sem <workers> report\n");
    exit(1);
}

// True if we're supposed to report what we find.
bool report = false;

// Total count we've found.
int global_count = 0;

// Fixed-sized array for holding the sequence.
#define MAX_WORKERS 128
#define MAX_VALUES 10000

int vList[MAX_VALUES];
int vCount = 0;

int nextWorkIndex = 0;
bool doneReading = false;

pthread_mutex_t countMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t workAvailable;
sem_t workMutex;

bool checkTriangle(int a, int b, int c)
{
    return (a + b > c) && (a + c > b) && (b + c > a);
}

// Read the list of values.
void readList()
{
    // Keep reading as many values as we can.
    int v;
    while (scanf("%d\n", &v) == 1)
    {
        // Make sure we have enough room, then store the latest input.
        if (vCount > MAX_VALUES)
            fail("Too many input values");

        // Store the latest value.
        vList[vCount++] = v;
    }
}

/** Return the index, b, of the value we're supposed to use (Checking it against all
    earlier values in the list). */
int getWork()
{
    sem_wait(&workAvailable);

    // Check before locking
    if (nextWorkIndex >= vCount)
        return -1; // No more work

    sem_wait(&workMutex);
    int index = nextWorkIndex++;
    sem_post(&workMutex);

    return index;
}

/** Start routine for each worker. */
void *workerRoutine(void *arg)
{
    pid_t thread_id = syscall(__NR_gettid);
    int local_count = 0;

    while (true)
    {
        int index = getWork();
        if (index == -1)
            break;

        for (int j = 0; j < index; j++)
        {
            for (int k = j + 1; k < index; k++)
            {
                if (checkTriangle(vList[j], vList[k], vList[index]))
                {
                    local_count++;

                    pthread_mutex_lock(&countMutex);
                    global_count++;
                    pthread_mutex_unlock(&countMutex);

                    if (report)
                    {
                        printf("I'm thread %d. Local count: %d. Triangle (%d, %d, %d) found at: %d, %d and %d.\n",
                               thread_id, local_count, vList[j], vList[k], vList[index], index, k, j);
                    }
                }
            }
        }

        sem_post(&workAvailable);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int workers = 4;

    // Parse command-line arguments.
    if (argc < 2 || argc > 3)
        usage();

    if (sscanf(argv[1], "%d", &workers) != 1 ||
        workers < 1)
        usage();

    // If there's a second argument, it better be "report"
    if (argc == 3)
    {
        if (strcmp(argv[2], "report") != 0)
            usage();
        report = true;
    }

    // Make each of the workers.
    pthread_t worker[workers];
    sem_init(&workAvailable, 0, 0);
    sem_init(&workMutex, 0, 1);

    for (int i = 0; i < workers; i++)
    {
        pthread_create(&worker[i], NULL, workerRoutine, NULL);
    }
    // Then, start getting work for them to do.
    readList();

    // Notify all workers to exit
    for (int i = 0; i < workers; i++)
    {
        sem_post(&workAvailable);
    }

    // Wait until all the workers finish.
    for (int i = 0; i < workers; i++)
    {
        pthread_join(worker[i], NULL);
    }

    sem_destroy(&workAvailable);
    sem_destroy(&workMutex);
    pthread_mutex_destroy(&countMutex);

    // Report the max and release the semaphores.
    printf("Total count: %d\n", global_count);

    return EXIT_SUCCESS;
}
