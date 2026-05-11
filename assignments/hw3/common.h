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

// Struct for string info in shared memory
typedef struct
{
    char string[STRING_LIMIT];
    char prev1[STRING_LIMIT];
    char prev2[STRING_LIMIT];
    char prev3[STRING_LIMIT];
    int len;
    int num_undos;
} EditState;

// Shared memory key
static key_t get_key()
{
    // For common platform
    key_t key = ftok("/mnt/ncsudrive/w/wrboone", 'E');
    
    // For my WSL
    // key_t key = ftok("/home/boonewr", 'E');
    if (key == -1)
    {
        perror("ftok failed");
        exit(1);
    }
    return key;
}
