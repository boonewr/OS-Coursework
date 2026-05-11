#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// Print out an error message and exit.
static void fail(char const *message)
{
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}

// Print a short usage message, then exit.
static void usage()
{
    fprintf(stderr, "usage: <host> <port> <path>\n");
    exit(EXIT_FAILURE);
}

/** Simple program to send a request of http1.0 and report the
    response. */
int main(int argc, char *argv[])
{
    if (argc != 4)
        usage();

    char *host = argv[1];
    char *port = argv[2];

    // For getaddrinfo(), tell the system what kinds of addresses we want
    struct addrinfo addrCriteria = {
        .ai_family = AF_UNSPEC,     // Use either IPV4 or IPV6
        .ai_socktype = SOCK_STREAM, // Use byte stream
        .ai_protocol = IPPROTO_TCP  // Use TCP
    };

    // Lookup a list of matching addresses
    struct addrinfo *servAddr;
    if (getaddrinfo(host, port, &addrCriteria, &servAddr) != 0)
        fail("Can't get address info");

    // Try to just use the first address we get back, make sure we got one.
    if (servAddr == NULL)
        fail("Can't get address");

    // Make a socket with parameters (e.g., IPV4 vs IPV6) from getaddrinfo()
    int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
                      servAddr->ai_protocol);
    if (sock < 0)
        fail("Can't create socket");

    // Connect this socket to the server.
    if (connect(sock, servAddr->ai_addr, servAddr->ai_addrlen) != 0)
        fail("Can't connect to server");

    // We're done wiht the address info now.
    freeaddrinfo(servAddr);

    // Construct the header for the request.
    char *path = argv[3];
    char request[1024];

    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s:%s\r\n"
             "User-Agent: CSC 246 class wrboone@ncsu.edu\r\n"
             "Accept: */*\r\n"
             "Connection: close\r\n"
             "\r\n",
             path, host, port);

    // Send the request to the server.
    if (send(sock, request, strlen(request), 0) < 0)
        fail("Failed to send request");

    // Read back and print out the response.
    char buffer[1024];
    ssize_t bytesRead;
    bool inHeader = true;

    while ((bytesRead = read(sock, buffer, sizeof(buffer))) > 0)
    {
        for (ssize_t i = 0; i < bytesRead; i++)
        {
            char c = buffer[i];

            if (inHeader)
            {
                fputc(c, stderr);

                static char last4[4] = {0};
                last4[0] = last4[1];
                last4[1] = last4[2];
                last4[2] = last4[3];
                last4[3] = c;

                if (last4[0] == '\r' && last4[1] == '\n' &&
                    last4[2] == '\r' && last4[3] == '\n')
                {
                    inHeader = false;
                }
            }
            else
            {
                fputc(c, stdout);
            }
        }
    }

    // Close the socket connection
    close(sock);

    return 0;
}
