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

#define MAX_WORDS 50

// Print out generic error message and exit.
static void fail()
{
    fprintf(stderr, "ERROR\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        fail();

    mqd_t serverQueue = mq_open(SERVER_QUEUE, O_WRONLY);
    mqd_t clientQueue = mq_open(CLIENT_QUEUE, O_RDONLY);

    struct ClientRequest request;

    char command = argv[1][0];

    switch (command)
    {
    case 'i':
        if (argc != 4)
            fail();
        if (strcmp(argv[1], "insert") != 0)
            fail();

        request.command = 'i';
        request.index = atoi(argv[2]);
        request.character = argv[3][0];
        break;
    case 'd':
        if (argc != 3)
            fail();
        if (strcmp(argv[1], "delete") != 0)
            fail();

        request.command = 'd';
        request.index = atoi(argv[2]);
        request.character = 1;
        break;
    case 'u':
        if (argc != 2)
            fail();
        if (strcmp(argv[1], "undo") != 0)
            fail();

        request.command = 'u';
        request.index = 0;
        request.character = 0;
        break;
    case 'r':
        if (argc != 2)
            fail();
        if (strcmp(argv[1], "report") != 0)
            fail();

        request.command = 'r';
        request.index = 0;
        request.character = 0;
        break;
    default:
        fail();
    }

    if (request.index < 0)
        fail();

    int sendCode = mq_send(serverQueue, (char *)&request, sizeof(struct ClientRequest), 0);
    if (sendCode == -1)
    {
        fail();
        mq_close(serverQueue);
        mq_close(clientQueue);
        return 1;
    }

    // struct to receive the response
    struct ServerResponse response;
    memset(&response, 0, sizeof(response));

    int receiveCode = mq_receive(clientQueue, (char *)&response, MESSAGE_LIMIT, NULL);
    if (receiveCode == -1)
    {
        fail();
        mq_close(serverQueue);
        mq_close(clientQueue);
        return 1;
    }

    response.message[STRING_LIMIT - 1] = '\0';
    printf("%s\n", response.message);

    // Close the queues
    mq_close(clientQueue);
    mq_close(serverQueue);

    return 0;
}