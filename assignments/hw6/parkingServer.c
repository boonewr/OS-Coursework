#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>

/** Port number used by my server */
#define PORT_NUMBER "26310"

/** Maximum command line length */
#define N 256

#define MAX_PLATE_LEN 9
typedef struct
{
    char plate[MAX_PLATE_LEN];
    int arrivalTime;
    bool occupied;
} ParkingRecord;

ParkingRecord *parkingTable;
int parkingCapacity;
float hourlyRate;
pthread_mutex_t tableLock = PTHREAD_MUTEX_INITIALIZER;

/** Print out an error message and exit. */
static void fail(char const *message)
{
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}

bool isValidPlate(char *plate)
{
    int len = strlen(plate);
    if (len < 7 || len > 8)
        return false;

    int dashCount = 0;
    for (int i = 0; i < len; i++)
    {
        if (plate[i] == '-')
        {
            dashCount++;
            if (dashCount > 1)
                return false;
        }
        else if (!isdigit(plate[i]) && !isupper(plate[i]))
        {
            return false;
        }
    }

    return true;
}

void formatTime(int seconds, char *buffer)
{
    int h = seconds / 3600;
    int m = (seconds % 3600) / 60;
    int s = seconds % 60;
    snprintf(buffer, 16, "%02d:%02d:%02d", h, m, s);
}

/** handle a client connection, close it when we're done. */
void *handleClient(void *arg)
{
    int sock = *(int *)arg;
    free(arg);
    // Here's a nice trick, wrap a C standard IO FILE around the
    // socket, so we can communicate the same way we would read/write
    // a file.
    FILE *fp = fdopen(sock, "a+");

    char cmd[N];

    // Prompt the user for a command.
    fprintf(fp, "cmd> ");

    fflush(fp); // Make sure it shows up immediately

    while (fgets(cmd, N, fp) != NULL)
    {
        // Strip newline
        char *newline = strpbrk(cmd, "\r\n");
        if (newline)
            *newline = '\0';

        if (strlen(cmd) == 0)
        {
            fprintf(fp, "cmd> ");
            fflush(fp);
            continue;
        }

        char *args[4]; // command + 2 args + NULL
        int argCount = 0;

        char *token = strtok(cmd, " ");
        while (token && argCount < 3)
        {
            args[argCount++] = token;
            token = strtok(NULL, " ");
        }

        args[argCount] = NULL;

        // for (int i = 0; i < argCount; i++)
        // {
        //     fprintf(fp, "  [%d]: '%s'\n", i, args[i]);
        // }

        if (argCount == 0)
        {
            fprintf(fp, "Invalid command\n");
        }
        else if (strcmp(args[0], "quit") == 0) // quit
        {
            break;
        }
        else if (strcmp(args[0], "park") == 0) // park
        {

            // args[1] = plate, args[2] = time
            if (argCount < 3)
            {
                fprintf(fp, "Invalid command\n");
            }
            else
            {
                char *plate = args[1];
                int arrival = atoi(args[2]);

                if (!isValidPlate(plate) || arrival <= 0 || arrival > 86400)
                {
                    fprintf(fp, "Invalid command\n");
                }
                else
                {
                    pthread_mutex_lock(&tableLock);

                    // Check for duplicate plate
                    bool duplicate = false;
                    for (int i = 0; i < parkingCapacity; i++)
                    {
                        if (parkingTable[i].occupied && strcmp(parkingTable[i].plate, plate) == 0)
                        {
                            duplicate = true;
                            break;
                        }
                    }

                    if (duplicate)
                    {
                        fprintf(fp, "Invalid command\n");
                    }
                    else
                    {
                        // Find a free slot
                        bool parked = false;
                        for (int i = 0; i < parkingCapacity; i++)
                        {
                            if (!parkingTable[i].occupied)
                            {
                                strncpy(parkingTable[i].plate, plate, MAX_PLATE_LEN - 1);
                                parkingTable[i].plate[MAX_PLATE_LEN - 1] = '\0';
                                parkingTable[i].arrivalTime = arrival;
                                parkingTable[i].occupied = true;
                                parked = true;
                                break;
                            }
                        }

                        if (!parked)
                        {
                            fprintf(fp, "Parking full - no available spaces.\n");
                        }
                    }

                    pthread_mutex_unlock(&tableLock);
                }
            }
        }
        else if (strcmp(args[0], "leave") == 0) // leave
        {
            // args[1] = plate, args[2] = time

            if (argCount < 3)
            {
                fprintf(fp, "Invalid command\n");
            }
            else
            {
                char *plate = args[1];
                int leaveTime = atoi(args[2]);

                if (!isValidPlate(plate) || leaveTime <= 0 || leaveTime > 86400)
                {
                    fprintf(fp, "Invalid command\n");
                }
                else
                {
                    pthread_mutex_lock(&tableLock);

                    bool found = false;
                    for (int i = 0; i < parkingCapacity; i++)
                    {
                        if (parkingTable[i].occupied &&
                            strcmp(parkingTable[i].plate, plate) == 0)
                        {

                            found = true;

                            if (leaveTime < parkingTable[i].arrivalTime)
                            {
                                fprintf(fp, "Invalid command\n");
                                break;
                            }

                            int duration = leaveTime - parkingTable[i].arrivalTime;
                            int hours = (duration + 3599) / 3600;

                            float fee = hours * hourlyRate;
                            float maxFee = hourlyRate * 10;
                            if (fee > maxFee)
                                fee = maxFee;

                            fprintf(fp, "Parking fee: $%.2f\n", fee);

                            // Clear record
                            parkingTable[i].occupied = false;
                            parkingTable[i].plate[0] = '\0';
                            parkingTable[i].arrivalTime = 0;
                            break;
                        }
                    }

                    if (!found)
                    {
                        fprintf(fp, "Plate number %s not found in the parking lot.\n", plate);
                    }

                    pthread_mutex_unlock(&tableLock);
                }
            }
        }
        else if (strcmp(args[0], "show") == 0)
        {
            pthread_mutex_lock(&tableLock);

            ParkingRecord *copy = malloc(parkingCapacity * sizeof(ParkingRecord));
            int count = 0;

            for (int i = 0; i < parkingCapacity; i++)
            {
                if (parkingTable[i].occupied)
                {
                    copy[count++] = parkingTable[i];
                }
            }

            for (int i = 0; i < count - 1; i++)
            {
                for (int j = i + 1; j < count; j++)
                {
                    if (copy[j].arrivalTime < copy[i].arrivalTime)
                    {
                        ParkingRecord temp = copy[i];
                        copy[i] = copy[j];
                        copy[j] = temp;
                    }
                }
            }

            fprintf(fp, "Plate Number    Time\n");

            for (int i = 0; i < count; i++)
            {
                char timeStr[16];
                formatTime(copy[i].arrivalTime, timeStr);
                fprintf(fp, "%-15s%s\n", copy[i].plate, timeStr);
            }

            free(copy);
            pthread_mutex_unlock(&tableLock);
        }
        else
        {
            fprintf(fp, "Invalid command\n");
        }

        fprintf(fp, "cmd> ");
        fflush(fp);
    }

    fclose(fp);
    return NULL;
}

int main(int argc, char *argv[])
{
    // Prepare a description of server address criteria.
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_INET;
    addrCriteria.ai_flags = AI_PASSIVE;
    addrCriteria.ai_socktype = SOCK_STREAM;
    addrCriteria.ai_protocol = IPPROTO_TCP;

    if (argc != 3)
    {
        fprintf(stderr, "usage: parkingServer <parking spaces> <hourly rate>\n");
        exit(EXIT_FAILURE);
    }
    parkingCapacity = atoi(argv[1]);
    hourlyRate = atof(argv[2]);

    if (parkingCapacity <= 0 || hourlyRate <= 0)
    {
        fprintf(stderr, "usage: parkingServer <parking spaces> <hourly rate>\n");
        exit(EXIT_FAILURE);
    }

    parkingTable = calloc(parkingCapacity, sizeof(ParkingRecord));
    if (!parkingTable)
        fail("Allocation failed");

    // Lookup a list of matching addresses
    struct addrinfo *servAddr;
    if (getaddrinfo(NULL, PORT_NUMBER, &addrCriteria, &servAddr))
        fail("Can't get address info");

    // Try to just use the first one.
    if (servAddr == NULL)
        fail("Can't get address");

    // Create a TCP socket
    int servSock = socket(servAddr->ai_family, servAddr->ai_socktype,
                          servAddr->ai_protocol);
    if (servSock < 0)
        fail("Can't create socket");

    int yes = 1;
    setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    // Bind to the local address
    if (bind(servSock, servAddr->ai_addr, servAddr->ai_addrlen) != 0)
        fail("Can't bind socket");

    // Tell the socket to listen for incoming connections.
    if (listen(servSock, 5) != 0)
        fail("Can't listen on socket");

    // Free address list allocated by getaddrinfo()
    freeaddrinfo(servAddr);

    // Fields for accepting a client connection.
    struct sockaddr_storage clntAddr; // Client address
    socklen_t clntAddrLen = sizeof(clntAddr);

    while (true)
    {
        // Accept a client connection.
        // int sock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
        // handleClient(sock);
        int *sockPtr = malloc(sizeof(int));
        *sockPtr = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
        if (*sockPtr < 0)
            continue;

        pthread_t tid;
        pthread_create(&tid, NULL, handleClient, sockPtr);
        pthread_detach(tid);
    }

    // Stop accepting client connections (never reached).
    close(servSock);

    return 0;
}
