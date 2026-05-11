#include <stdio.h>
#include <stdlib.h>

// Make a short name for a Node.
typedef struct Node Node;

// Node to build our list from.
struct Node
{
    // Value in our list
    int val;

    // Pointer to the next node.
    Node *next;
};

// Representation for a list, with a head and a tail pointer.
typedef struct
{
    // Head pointer for the list.
    Node *head;

    // Pointer to the last node in the list, or null if the list
    // is empty.
    Node *tail;
} List;

// Recursive quicksort.
void sort(List *list)
{
    if (list->head == NULL || list->head->next == NULL)
    {
        return;
    }

    Node *pivot = list->head;
    Node *curr = pivot->next;
    Node *next = NULL;

    List lesser = {NULL, NULL};
    List greater = {NULL, NULL};

    while (curr)
    {
        next = curr->next;
        curr->next = NULL;

        if (curr->val < pivot->val)
        {
            if (lesser.tail)
                lesser.tail->next = curr;
            else
                lesser.head = curr;
            lesser.tail = curr;
        }
        else
        {
            if (greater.tail)
                greater.tail->next = curr;
            else
                greater.head = curr;
            greater.tail = curr;
        }

        curr = next;
    }

    // Recurse on each partition
    sort(&lesser);
    sort(&greater);

    // Lesser + pivot + greater
    if (lesser.head)
    {
        list->head = lesser.head;
        lesser.tail->next = pivot;
    }
    else
    {
        list->head = pivot;
    }

    pivot->next = greater.head;

    if (greater.tail)
        list->tail = greater.tail;
    else
        list->tail = pivot;
}

int main(int argc, char *argv[])
{
    int n = 10;
    if (argc >= 2)
        if (argc > 2 || sscanf(argv[1], "%d", &n) != 1 ||
            n < 0)
        {
            fprintf(stderr, "usage: array <n>\n");
            exit(EXIT_FAILURE);
        }

    // Representaton for the list.
    List list = {NULL, NULL};

    // The tail pointer in List makes it easy to grow the list
    // front-to-back.
    for (int i = 0; i < n; i++)
    {
        // Make a node containing a random value.
        Node *n = (Node *)malloc(sizeof(struct Node));
        *n = (Node){rand(), NULL};

        // Link it in at the tail end of the list.
        if (list.tail)
            list.tail->next = n;
        else
            list.head = n;
        list.tail = n;
    }

    sort(&list);

    // Print the sorted items.
    for (Node *n = list.head; n; n = n->next)
        printf("%d\n", n->val);

    // Free memory for the list.
    while (list.head)
    {
        Node *n = list.head;
        list.head = n->next;
        free(n);
    }

    return 0;
}
