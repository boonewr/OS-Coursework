#include <stdio.h>
#include <unistd.h>  // for write
#include <pthread.h> // for pthreads
#include <stdlib.h>  // for exit

// Print out an error message and exit.
static void fail(char const *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

// Define the start routines for your three threads.  Each thread will
// use two calls to write() to print its two characters, then it will
// terminate.

// Start routine for the first thread
void *startRoutine1(void *arg)
{
    write(STDOUT_FILENO, "a", 1);
    write(STDOUT_FILENO, "b", 1);
    pthread_exit(NULL);
}

// Start routine for the second thread
void *startRoutine2(void *arg)
{
    write(STDOUT_FILENO, "c", 1);
    write(STDOUT_FILENO, "d", 1);
    pthread_exit(NULL);
}

// Start routine for the third thread
void *startRoutine3(void *arg)
{
    write(STDOUT_FILENO, "e", 1);
    write(STDOUT_FILENO, "f", 1);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // A bunch of times.
    for (int i = 0; i < 100000; i++)
    {
        // Make three threads.
        pthread_t thread1, thread2, thread3;
        if (pthread_create(&thread1, NULL, startRoutine1, NULL) != 0)
            fail("Can't create thread1");
        if (pthread_create(&thread2, NULL, startRoutine2, NULL) != 0)
            fail("Can't create thread2");
        if (pthread_create(&thread3, NULL, startRoutine3, NULL) != 0)
            fail("Can't create thread3");

        // Join with the three threads.
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);
        pthread_join(thread3, NULL);

        // Use the write system call to print out a newline.  The string
        // we're passing to write is null terminated (since that's what
        // double quotesd strings are in C), but we're just using the
        // first byte (the newline).  Write doesn't care about null
        // terminated strings, it just writes out any sequence of bytes
        // you ask it to.
        write(STDOUT_FILENO, "\n", 1);
    }

    return 0;
}
