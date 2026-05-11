/**
 * @file stash.c
 *
 * @author Robbie Boone (wrboone)
 *
 * This program is a simple shell for running commands. It has build in functionality
 * for exiting and changing directories, and can run other commands as well through execvp().
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

/** Maximum characters in an input command */
#define MAX_COMMAND_LEN 1024
/** Maximum number of words in a command */
#define MAX_WORDS 514

/** Global variable to store ID of the last child created by runCommand in the background for EC */
pid_t older_sibling = -1;

/**
 * Prints an error message for an invalid command.
 */
void invalid()
{
    printf("Invalid command\n");
}

/**
 * This function parses a line representing a user command as input. It breaks the line into
 * individual null-terminated words for the rest of the program to use.
 *
 * @param line The line to parse.
 * @param words The array to store the words in.
 * @return The number of words in the line.
 */
int parseCommand(char *line, char *words[])
{
    int count = 0;
    char str[100];
    int offset;

    while (count < MAX_WORDS && sscanf(line, "%99s%n", str, &offset) == 1)
    {
        words[count] = (char *)malloc(strlen(str) + 1);
        strcpy(words[count], str);
        count++;
        line += offset;
    }
    return count;
}

/**
 * This function performs the built-in exit command. Exits with the status given in
 * the input command.
 *
 * @param words The array of words in the command.
 * @param count The number of words in the command.
 */
void runExit(char *words[], int count)
{
    int n = 0;
    int status = 0;
    n = sscanf(words[1], "%d", &status);

    if (n < 1 || count != 2)
    {
        invalid();
        return;
    }

    exit(status);
}

/**
 * This function performs the built-in cd command. Changes the current working directory
 * to the one specified in the given command.
 * 
 * @param words The array of words in the command.
 * @param count The number of words in the command.
 * 
 */
void runCd(char *words[], int count)
{
    if (count != 2)
    {
        invalid();
        return;
    }

    int result = chdir(words[1]);

    if (result != 0)
    {
        invalid();
    }
}

/**
 * This function runs a (non-built-in) command by creating a child process and having it call
 * execvp() to run the given command.
 * 
 * @param words The array of words in the command.
 * @param count The number of words in the command.
 */
void runCommand(char *words[], int count)
{
    int background = 0;
    if (words[count - 1][0] == '&')
    {
        background = 1;
        count--;
    }

    if (older_sibling > 0)
    {
        /*
        Not sure if this is the intended solution. I was searching around for
        monitoring children, and found this here:
        https://linux.die.net/man/2/waitpid
        */
        pid_t result = waitpid(older_sibling, NULL, WNOHANG);
        if (result == older_sibling)
        {
            printf("[%d done]\n", older_sibling);
            older_sibling = -1;
        }
    }

    pid_t id = fork();

    if (id == -1)
        printf("Can't create child\n");

    words[count] = NULL;

    if (id == 0)
    { // Child
        execvp(words[0], words);
        printf("Can't run command %s\n", words[0]);
    }
    else
    { // Parent
        if (background)
        {
            older_sibling = id;
            printf("id [%d]\n", id);
            fflush(stdout);
        }
        else
        {
            waitpid(id, NULL, 0);
        }
    }
}

/**
 * Main function for running the shell. Presents a 'stash> ' prompt and continuously accepts
 * new commands until exit.
 */
int main()
{
    char *prompt = "stash> ";
    printf("%s", prompt);
    char command[MAX_COMMAND_LEN] = "";

    char *commands[MAX_WORDS];
    int args = 0;

    // while (fgets(command, sizeof(command), stdin) && command[0] != '\n')
    while (fgets(command, sizeof(command), stdin))
    {
        if (command[0] == '\n')
        {
            printf("%s", prompt);
            continue;
        }

        args = parseCommand(command, commands);

        if (strcmp(commands[0], "cd") == 0)
        {
            runCd(commands, args);
        }
        else if (strcmp(commands[0], "exit") == 0)
        {
            runExit(commands, args);
        }
        else
        {
            runCommand(commands, args);
        }

        // Issue: for fast commands like ls that get run in the background, the prompt is printed before
        // the child is done, resulting in the output of the command printing after the prompt
        printf("%s", prompt);

        for (int i = 0; i < args; i++)
        {
            free(commands[i]);
        }
    }

    return EXIT_SUCCESS;
}
