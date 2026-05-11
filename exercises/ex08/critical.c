#include <stdio.h>
#include <pthread.h> // for pthreads
#include <stdlib.h>  // for exit
#include <stdbool.h> // for bool
#include <unistd.h>  // for sleep

// Number of linked list nodes to make.
#define LIST_LEN 100

int lock = 0;

// Print out an error message and exit.
static void fail(char const *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

// Helper function to loop and wait for the lock to release
static void lockTestAndSet(int *lock)
{
    while (__sync_lock_test_and_set(lock, 1))
    {
        // Wait
        ;
    }
}

// Node structure.  We're making a linked list out of these.
struct Node
{
    // Counter value for the number of times this node
    // gets removed from the linked list and placed at the
    // tail.
    int counter;

    // pointer to the next node.
    struct Node *next;
};

// Head of our linked list
static struct Node *head = NULL;

// Tail pointer for our linked list.
static struct Node *tail = NULL;

// True until it's time to exit.
static bool running = true;

/** Extract a node from the head of the linked list and return it. */
struct Node *dequeueNode()
{
    // Critical section entry code.
    lockTestAndSet(&lock);

    // Get a node from the head of the linked list.
    struct Node *node = head;
    head = head->next;

    // Critical section exit code.
    __sync_lock_release(&lock);

    return node;
}

/** Enqueue the given node at the tail of the linked list. */
void enqueueNode(struct Node *node)
{
    // Critical section entry code.
    lockTestAndSet(&lock);

    // Put the node on the tail of the list.
    tail->next = node;
    tail = node;
    node->next = NULL;

    // Critical section exit code.
    __sync_lock_release(&lock);
}

/** Thread routine to repeatedly dequeue and then
    enqueue nodes. */
void *cycleList(void *arg)
{
    // Keep running until the thread in main tells us to stop.
    while (running)
    {
        struct Node *node = dequeueNode();

        // Increment its counter in this node.  This isn't part of a
        // critical section.  We're the only thread that will be
        // incrementing the count on this particular node, so we shouldn't
        // get any race conditions.
        node->counter++;

        enqueueNode(node);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    // Number of threads to make.
    int tcount = 1;

    // On the command line, they can tell us how many threads.
    if (argc > 1 && sscanf(argv[1], "%d", &tcount) != 1)
        fail("usage: critical <number-of-threads>");

    // Put a bunch of nodes on our linked list.
    for (int i = 0; i < LIST_LEN; i++)
    {
        // Make a new node.
        struct Node *node = (struct Node *)malloc(sizeof(struct Node));
        node->counter = 0;

        // If the list was empty, it's the new tail.
        if (tail == NULL)
            tail = node;
        // Link it in as the new head node.
        node->next = head;
        head = node;
    }

    // Make the number of threads given on the command-line
    pthread_t thread[tcount];
    for (int i = 0; i < tcount; i++)
        if (pthread_create(thread + i, NULL, cycleList, NULL) != 0)
            fail("Can't create a new thread\n");

    // Let the threads run for 5 seconds.
    sleep(5);

    // Ask them to stop
    running = false;

    // Join with all of them.
    for (int i = 0; i < tcount; i++)
        pthread_join(thread[i], NULL);

    // Count number of nodes (should be unchanged) and the total count
    // across all of them (an indication of how much work our threads
    // were able to do.)
    int nodes = 0;
    long total = 0;
    for (struct Node *p = head; p; p = p->next)
    {
        nodes += 1;
        total += p->counter;
    }

    // Print a report
    printf("nodes: %d\ttotal count: %ld\n", nodes, total);

    return 0;
}
