
/* common.h */
#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maximum length of the string being edited.
#define STRING_LIMIT 76

// Number of undo operations to store.
#define UNDO_LIMIT 3

// Maximum length of the string being edited.
#define STRING_LIMIT 76

// Number of undo operations to store.
#define UNDO_LIMIT 3

// Types of command
typedef enum
{
    INSERT,
    DELETE,
    NONE
} CommandType;

// Structure for an undo record
typedef struct
{
    CommandType type;
    int index;
    char character;
} UndoRecord;

// Structure for shared memory
typedef struct
{
    char string[STRING_LIMIT];
    UndoRecord history[UNDO_LIMIT];
    int undo_count;
} EditState;

// Shared memory key
static key_t get_key()
{
    key_t key = ftok("/mnt/ncsudrive/w/wrboone", 'E');
    if (key == -1)
    {
        perror("ftok failed");
        exit(1);
    }
    return key;
}

#endif /* COMMON_H */
