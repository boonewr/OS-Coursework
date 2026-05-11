#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

// Number of iterations for each thread
#define ITERATIONS 500

// Who gets to go next.
int nextTurn = 0;

// Print out an error message and exit.
static void fail(char const *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

// Start routines for the two threads.

void *pingRoutine(void *arg)
{
    for (int i = 0; i < ITERATIONS; i++)
    {
        while (nextTurn != 0)
        {
            ;
        }
        nextTurn = 1;
    }
}

void *pongRoutine(void *arg)
{
    for (int i = 0; i < ITERATIONS; i++)
    {
        while (nextTurn != 1)
        {
            ;
        }
        nextTurn = 0;
    }
}

int main(int argc, char *argv[])
{

    // Create each of the two threads.
    pthread_t pingThread, pongThread;
    if (pthread_create(&pingThread, NULL, pingRoutine, NULL) != 0)
        fail("Failed to create ping thread");

    if (pthread_create(&pongThread, NULL, pongRoutine, NULL) != 0)
        fail("Failed to create pong thread");

    // Wait for them both to finish.
    if (pthread_join(pingThread, NULL) != 0)
        fail("Failed to join ping thread");

    if (pthread_join(pongThread, NULL) != 0)
        fail("Failed to join pong thread");

    return 0;
}
