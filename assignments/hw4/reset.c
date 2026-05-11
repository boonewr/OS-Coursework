#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>
#include "common.h"

// Print out an error message and exit.
static void fail(char const *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

// Print out a usage message and exit.
static void usage()
{
    fprintf(stderr, "usage: reset INITIAL-STRING\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        usage();
    }

    // Get starting string
    const char *initial_string = argv[1];
    if (strlen(initial_string) >= STRING_LIMIT)
        fail("Input string too long");

    // Create shared memory
    key_t SHM_KEY = get_key();
    int shmid = shmget(SHM_KEY, sizeof(EditState), IPC_CREAT | 0666);
    if (shmid == -1)
    {
        fail("shmget failed");
    }

    // Attach shared memory
    EditState *state = (EditState *)shmat(shmid, NULL, 0);
    if (state == (void *)-1)
    {
        fail("shmat failed");
    }

    // Initialize shared memory
    strncpy(state->string, initial_string, STRING_LIMIT);
    state->string[STRING_LIMIT - 1] = '\0';
    state->len = strlen(initial_string);
    state->num_undos = 0;

    sem_t *sem = sem_open(LOCK_NAME, O_CREAT, 0666, 1); 
    if (sem == SEM_FAILED)
    {
        fail("sem_open failed");
    }

    // Detach shared memory
    if (shmdt(state) == -1)
    {
        fail("shmdt failed");
    }

    return 0;
}
