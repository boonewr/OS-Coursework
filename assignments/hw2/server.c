#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "common.h"

// Print out an error message and exit.
static void fail(char const *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

static void usage()
{
    fail("usage: server INITIAL-STRING");
}

// Flag for telling the server to stop running because of a sigint.
// This is safer than trying to print in the signal handler.
static volatile int running = 1;

/**
 * Signal handler for SIGINT. Enables printing of final string when user stops server running.
 */
void sigHandler(int sig)
{
    running = 0;
}

int main(int argc, char *argv[])
{
    struct sigaction act;
    act.sa_handler = sigHandler;
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);

    // Remove both queues, in case, last time, this program terminated
    // abnormally with some queued messages still queued.
    mq_unlink(SERVER_QUEUE);
    mq_unlink(CLIENT_QUEUE);

    // Prepare structure indicating maximum queue and message sizes.
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MESSAGE_LIMIT;

    // Make both the server and client message queues.
    mqd_t serverQueue = mq_open(SERVER_QUEUE, O_RDONLY | O_CREAT, 0600, &attr);
    mqd_t clientQueue = mq_open(CLIENT_QUEUE, O_WRONLY | O_CREAT, 0600, &attr);
    if (serverQueue == -1 || clientQueue == -1)
        fail("Can't create the needed message queues");

    if (argc != 2)
        usage();

    struct EditingString
    {
        char string[STRING_LIMIT];
        char prev1[STRING_LIMIT];
        char prev2[STRING_LIMIT];
        char prev3[STRING_LIMIT];
        int len;
        int num_undos;
    };

    // struct for holding the working string
    struct EditingString argStr;
    strncpy(argStr.string, argv[1], STRING_LIMIT - 1);
    argStr.string[STRING_LIMIT - 1] = '\0';
    argStr.len = strlen(argv[1]);
    argStr.num_undos = 0;

    if (strchr(argStr.string, '\n') != NULL)
        usage();

    // Repeatedly read and process client messages.
    int receiveCode = -1;
    struct ClientRequest request;
    struct ServerResponse response;
    // status 0: OK
    // status 1: Error
    // status 2: return string
    response.status = 0;
    while (running)
    {
        argStr.num_undos > 3 ? argStr.num_undos = 3 : argStr.num_undos;
        argStr.len = strlen(argStr.string);
        response.status = 0;
        receiveCode = mq_receive(serverQueue, (char *)&request, MESSAGE_LIMIT, NULL);
        if (receiveCode == -1)
        {
            running = 0;
        }

        switch (request.command)
        {
        // insert idx c
        case 'i':
            if (request.index > argStr.len)
            {
                response.status = 1;
                break;
            }

            strcpy(argStr.prev3, argStr.prev2);
            strcpy(argStr.prev2, argStr.prev1);
            strcpy(argStr.prev1, argStr.string);
            for (int i = argStr.len; i > request.index; i--)
            {
                argStr.string[i] = argStr.string[i - 1];
            }
            argStr.string[request.index] = request.character;
            argStr.num_undos++;
            break;

        // delete idx
        case 'd':
            if (request.index > argStr.len)
            {
                response.status = 1;
                break;
            }
            strcpy(argStr.prev3, argStr.prev2);
            strcpy(argStr.prev2, argStr.prev1);
            strcpy(argStr.prev1, argStr.string);
            for (int i = request.index; i < STRING_LIMIT - 1; i++)
            {
                argStr.string[i] = argStr.string[i + 1];
            }
            argStr.num_undos++;
            break;

        // undo
        case 'u':
            if (argStr.num_undos > 0)
            {
                strcpy(argStr.string, argStr.prev1);
                strcpy(argStr.prev1, argStr.prev2);
                strcpy(argStr.prev2, argStr.prev3);
                argStr.num_undos--;
            }
            else
            {
                response.status = 1;
            }
            break;

        // report
        case 'r':
            response.status = 2;
            break;
        }

        switch (response.status)
        {
        case 0:
            strncpy(response.message, "OK", STRING_LIMIT - 1);
            break;
        case 1:
            strncpy(response.message, "ERROR", STRING_LIMIT - 1);
            break;
        case 2:
            char temp[STRING_LIMIT] = {};
            snprintf(temp, strlen(argStr.string) + 5, "%s [%d]", argStr.string, argStr.num_undos);
            strncpy(response.message, temp, STRING_LIMIT - 1);
            break;
        }

        response.message[STRING_LIMIT - 1] = '\0';

        int sendCode = mq_send(clientQueue, (char *)&response, sizeof(struct ServerResponse), 0);
        if (sendCode == -1)
        {
            running = 0;
        }
    }

    // Close our two message queues (and delete them).
    mq_close(clientQueue);
    mq_close(serverQueue);

    mq_unlink(SERVER_QUEUE);
    mq_unlink(CLIENT_QUEUE);

    // print out the final string and undos
    printf("\n%s [%d]\n", argStr.string, argStr.num_undos);

    return 0;
}
