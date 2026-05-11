#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <semaphore.h>
#include "common.h"

sem_t *sem;

// Print out an error message and exit.
static void fail(char const *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

// Helper function to check if a string is an integer.
bool strIsInt(const char *str)
{
    while (*str)
    {
        if (!isdigit((unsigned char)*str))
            return false;
        str++;
    }
    return true;
}

// Insert character ch at index idx
bool insert(EditState *state, int idx, char ch)
{

#ifndef UNSAFE
    sem_wait(sem);
#endif

    if (idx < 0 || idx > state->len || state->len >= STRING_LIMIT - 1)
    {
#ifndef UNSAFE
        sem_post(sem);
#endif
        return false;
    }

    strncpy(state->prev3, state->prev2, STRING_LIMIT);
    strncpy(state->prev2, state->prev1, STRING_LIMIT);
    strncpy(state->prev1, state->string, STRING_LIMIT);

    memmove(&state->string[idx + 1], &state->string[idx], state->len - idx + 1);
    state->string[idx] = ch;
    state->len++;
    if (state->num_undos < UNDO_LIMIT)
        state->num_undos++;

#ifndef UNSAFE
    sem_post(sem);
#endif
    return true;
}

// Delete the character at index idx
bool delete(EditState *state, int idx)
{
#ifndef UNSAFE
    sem_wait(sem);
#endif
    if (idx < 0 || idx >= state->len)
    {
#ifndef UNSAFE
        sem_post(sem);
#endif
        return false;
    }

    strncpy(state->prev3, state->prev2, STRING_LIMIT);
    strncpy(state->prev2, state->prev1, STRING_LIMIT);
    strncpy(state->prev1, state->string, STRING_LIMIT);

    memmove(&state->string[idx], &state->string[idx + 1], state->len - idx);
    state->len--;
    if (state->num_undos < UNDO_LIMIT)
        state->num_undos++;

#ifndef UNSAFE
    sem_post(sem);
#endif
    return true;
}

// Undo the most recent edit.
bool undo(EditState *state)
{
#ifndef UNSAFE
    sem_wait(sem);
#endif
    if (state->num_undos == 0)
    {
#ifndef UNSAFE
        sem_post(sem);
#endif
        return false;
    }

    strncpy(state->string, state->prev1, STRING_LIMIT);
    strncpy(state->prev1, state->prev2, STRING_LIMIT);
    strncpy(state->prev2, state->prev3, STRING_LIMIT);
    state->num_undos--;

#ifndef UNSAFE
    sem_post(sem);
#endif
    return true;
}

// Print the state of the string and the length of the undo history.
void report(EditState *state)
{
    printf("%s [%d]\n", state->string, state->num_undos);
}

// Test function for stress testing mutual exclusion
void test(EditState *state, int n, int idx, char ch)
{
    for (int i = 0; i < n; i++)
    {
        insert(state, idx, ch);
        delete (state, idx);
    }
}

int main(int argc, char *argv[])
{
    // Eventual exit status for success or failure.
    // int status = EXIT_FAILURE;

    if (argc < 2)
    {
        fail("usage: edit COMMAND [ARGS]");
    }

    // Attach shared memory
    key_t SHM_KEY = get_key();
    int shmid = shmget(SHM_KEY, sizeof(EditState), 0666);
    if (shmid == -1)
    {
        fail("shmget failed");
    }

    // Struct for shared string
    EditState *state = (EditState *)shmat(shmid, NULL, 0);
    if (state == (void *)-1)
    {
        fail("shmat failed");
    }

    sem = sem_open(LOCK_NAME, 0);
    if (sem == SEM_FAILED)
    {
        fail("sem_open failed");
    }

    if (strcmp(argv[1], "insert") == 0)
    {
        if (argc != 4 || strlen(argv[3]) != 1 || !strIsInt(argv[2]))
        {
            fail("usage: edit insert IDX CHAR");
        }
        int idx = atoi(argv[2]);
        char c = argv[3][0];

        printf(insert(state, idx, c) ? "OK\n" : "Error\n");
    }
    else if (strcmp(argv[1], "delete") == 0)
    {
        if (argc != 3 || !strIsInt(argv[2]))
        {
            fail("usage: edit delete IDX");
        }
        int idx = atoi(argv[2]);

        printf(delete (state, idx) ? "OK\n" : "Error\n");
    }
    else if (strcmp(argv[1], "undo") == 0)
    {
        printf(undo(state) ? "OK\n" : "Error\n");
    }
    else if (strcmp(argv[1], "report") == 0)
    {
        report(state);
    }
    else if (strcmp(argv[1], "test") == 0)
    {
        if (argc != 5 || !strIsInt(argv[2]) || !strIsInt(argv[3]) || strlen(argv[4]) != 1)
        {
            fail("usage: edit test N IDX CHAR");
        }
        int n = atoi(argv[2]);
        int idx = atoi(argv[3]);
        char c = argv[4][0];
        test(state, n, idx, c);
        printf("OK\n");
    }
    else
    {
        fail("Unknown command");
    }

    // Detach shared memory
    if (shmdt(state) == -1)
    {
        fail("shmdt failed");
    }

    return EXIT_SUCCESS;
}
