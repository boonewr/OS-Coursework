#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include "common.h"

// Print out an error message and exit.
static void fail(char const *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

int main(int argc, char *argv[])
{
    // Eventual exit status for success or failure.
    int status = EXIT_FAILURE;

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

    EditState *state = (EditState *)shmat(shmid, NULL, 0);
    if (state == (void *)-1)
    {
        fail("shmat failed");
    }

    if (strcmp(argv[1], "insert") == 0)
    {
        if (argc != 4)
        {
            fail("usage: edit insert IDX CHAR");
        }
        int idx = atoi(argv[2]);
        char c = argv[3][0];
        int len = strlen(state->string);

        if (idx < 0 || idx > len || len >= STRING_LIMIT - 1)
        {
            printf("Error\n");
        }
        else
        {
            memmove(&state->string[idx + 1], &state->string[idx], len - idx + 1);
            state->string[idx] = c;
            state->history[state->undo_count % UNDO_LIMIT] = (UndoRecord){INSERT, idx, c};
            if (state->undo_count < UNDO_LIMIT)
            {
                state->undo_count++;
            }
            printf("OK\n");
        }
    }
    else if (strcmp(argv[1], "delete") == 0)
    {
        if (argc != 3)
        {
            fail("usage: edit delete IDX");
        }
        int idx = atoi(argv[2]);
        int len = strlen(state->string);

        if (idx < 0 || idx >= len)
        {
            printf("Error\n");
        }
        else
        {
            char removed = state->string[idx];
            memmove(&state->string[idx], &state->string[idx + 1], len - idx);
            state->history[state->undo_count % UNDO_LIMIT] = (UndoRecord){DELETE, idx, removed};
            if (state->undo_count < UNDO_LIMIT)
            {
                state->undo_count++;
            }
            printf("OK\n");
        }
    }
    else if (strcmp(argv[1], "undo") == 0)
    {
        if (state->undo_count == 0)
        {
            printf("Error\n");
        }
        else
        {
            state->undo_count--;
            UndoRecord last = state->history[state->undo_count % UNDO_LIMIT];
            if (last.type == INSERT)
            {
                memmove(&state->string[last.index], &state->string[last.index + 1], strlen(state->string) - last.index);
            }
            else if (last.type == DELETE)
            {
                memmove(&state->string[last.index + 1], &state->string[last.index], strlen(state->string) - last.index + 1);
                state->string[last.index] = last.character;
            }
            printf("OK\n");
        }
    }
    else if (strcmp(argv[1], "report") == 0)
    {
        printf("%s [%d]\n", state->string, state->undo_count);
    }
    else
    {
        fail("Unknown command");
    }

    if (shmdt(state) == -1)
    {
        fail("shmdt failed");
    }

    return status;
}
