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
#include "common.h"

// Print out an error message and exit.
static void fail(char const *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

// Helper function to check if a string is an integer.
int strIsInt(char *str)
{
    int result = 1;
    while (*str)
    {
        if (!isdigit((char)*str))
            result = 0;
        str++;
    }
    return result;
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

    // Struct for shared string
    EditState *state = (EditState *)shmat(shmid, NULL, 0);
    if (state == (void *)-1)
    {
        fail("shmat failed");
    }

    // Insert command
    if (strcmp(argv[1], "insert") == 0)
    {
        if (argc != 4 || strlen(argv[3]) != 1 || !strIsInt(argv[2]))
        {
            fail("usage: edit insert IDX CHAR");
        }
        int idx = atoi(argv[2]);
        char c = argv[3][0];
        int len = state->len;

        if (idx < 0 || idx > len || len >= STRING_LIMIT - 1)
        {
            printf("Error\n");
        }
        else
        {
            // Add to undo history
            strncpy(state->prev3, state->prev2, STRING_LIMIT);
            strncpy(state->prev2, state->prev1, STRING_LIMIT);
            strncpy(state->prev1, state->string, STRING_LIMIT);

            memmove(&state->string[idx + 1], &state->string[idx], len - idx + 1);
            state->string[idx] = c;
            state->len++;
            if (state->num_undos < UNDO_LIMIT)
                state->num_undos++;
            printf("OK\n");
        }
    }
    // Delete command
    else if (strcmp(argv[1], "delete") == 0)
    {
        if (argc != 3 || !strIsInt(argv[2]))
        {
            fprintf(stderr, "usage: edit delete IDX\n");
            exit(1);
        }
        int idx = atoi(argv[2]);
        int len = state->len;

        if (idx < 0 || idx >= len)
        {
            printf("Error\n");
        }
        else
        {
            strncpy(state->prev3, state->prev2, STRING_LIMIT);
            strncpy(state->prev2, state->prev1, STRING_LIMIT);
            strncpy(state->prev1, state->string, STRING_LIMIT);

            memmove(&state->string[idx], &state->string[idx + 1], len - idx);
            state->len--;
            if (state->num_undos < UNDO_LIMIT)
                state->num_undos++;
            printf("OK\n");
        }
    }
    // Undo command
    else if (strcmp(argv[1], "undo") == 0)
    {
        if (state->num_undos == 0)
        {
            printf("Error\n");
        }
        else
        {
            // Roll back undo history
            strncpy(state->string, state->prev1, STRING_LIMIT);
            strncpy(state->prev1, state->prev2, STRING_LIMIT);
            strncpy(state->prev2, state->prev3, STRING_LIMIT);
            state->num_undos--;
            printf("OK\n");
        }
    }
    // Report command
    else if (strcmp(argv[1], "report") == 0)
        printf("%s [%d]\n", state->string, state->num_undos);
    else
    {
        fprintf(stderr, "Unknown command\n");
        exit(1);
    }

    // Detach shared memory
    if (shmdt(state) == -1)
    {
        fail("shmdt failed");
    }

    return status;
}
