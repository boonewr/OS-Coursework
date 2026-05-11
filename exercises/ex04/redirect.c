#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

static void fail(char *msg)
{
    // This program does redirection of standard output.  Good thing
    // we still have standard error around, so we can print error
    // messages even
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    // Make a child process to run cat.
    pid_t pid = fork();
    if (pid == -1)
        fail("Can't create child process");

    if (pid == 0)
    {
        // I'm the child.  Before replacing myself with the cat program,
        // change its environment so it reads from "input.txt" instead of
        // standard input.
        int fd = open("input.txt", O_RDONLY);
        if (fd == -1)
            fail("Error opening input.txt");

        if (dup2(fd, STDIN_FILENO) != 0)
            fail("Error replacing standard input");

        close(fd);

        fd = open("output.txt", O_WRONLY | O_CREAT, 0600);
        if (fd == -1)
            fail("Error opening output.txt");

        if (dup2(fd, STDOUT_FILENO) == -1)
            fail("Error replacing standard output");

        close(fd);

        // Now, run cat.  Even though it thinks it's reading from standard
        // input and writing to standard output, it will really be copying these
        // files.
        execl("/bin/cat", "cat", NULL);
        exit(EXIT_FAILURE);
    }

    // Wait for the cat program to finish.
    wait(NULL);

    return EXIT_SUCCESS;
}
