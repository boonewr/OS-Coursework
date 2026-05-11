// Name for the queue of messages going to the server.
#define SERVER_QUEUE "/wrboone-server-queue"

// Name for the queue of messages going to the current client.
#define CLIENT_QUEUE "/wrboone-client-queue"

// Maximum length for a message in the queue
// (Long enough to hold any server request or response)
#define MESSAGE_LIMIT 1024

// Maximum length of the string being edited.
#define STRING_LIMIT 76

// Number of undo operations to store.
#define UNDO_LIMIT 3

/**
 * Struct for client requests that will be sent to the server.
 *
 * Command codes:
 * i: Insert
 * d: delete
 * u: undo
 * r: report
 */
struct ClientRequest
{
    char command;
    int index;
    char character;
};

/**
 * Struct for server responses that will be sent to the client.
 *
 * Status codes:
 * 0: OK
 * 1: Error
 * 2: Return string with undos
 */
struct ServerResponse
{
    int status;
    char message[STRING_LIMIT];
};
