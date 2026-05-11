#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static void fail(char *msg)
{
    printf("Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    // Store a unique ID to track each child individually
    int pid1 = -1;
    int pid2 = -1;
    int pid3 = -1;

    for (int i = 0; i < 3; i++)
    {
        // Use this common ID just to distinguish the parent
        int pid = fork();

        if (pid == -1)
            fail("Can't create child process");

        if (pid == 0) // Child
        {
            if (pid1 == -1)
                pid1 = getpid();
            else if (pid2 == -1)
                pid2 = getpid();
            else if (pid3 == -1)
                pid3 = getpid();

            sleep(1);
            printf("I am %d, child of %d\n", getpid(), getppid());
            exit(EXIT_SUCCESS);
        }
    }

    // Wait for all children to finish
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);
    printf("Done\n");

    return EXIT_SUCCESS;
}
