#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdlib.h>

// Number of iterations for each thread
#define ITERATIONS 500

// Declare two anonymous semaphores, one to let each of the threads
// go next.
sem_t pingNext, pongNext;

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
        sem_wait(&pingNext);
        sem_post(&pongNext);
    }
    return NULL;
}

void *pongRoutine(void *arg)
{
    for (int i = 0; i < ITERATIONS; i++)
    {
        sem_wait(&pongNext);
        sem_post(&pingNext);
    }
    return NULL;
}

int main(int argc, char *argv[])
{

    // Create two semephaores, one to let ping go next and one to let
    // pong go next.

    sem_init(&pingNext, 0, 1);
    sem_init(&pongNext, 0, 0);

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
        
    // Destroy the two semaphores.
    sem_destroy(&pingNext);
    sem_destroy(&pongNext);

    return 0;
}
