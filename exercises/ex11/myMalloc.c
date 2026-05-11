#include "myMalloc.h"
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

/** Representation of a free memory hole. */
typedef struct HoleStruct
{
    // Size of this hole (not counting the size field itself).
    size_t size;

    // This takes zero bytes in the struct, but we can use it to easily get
    // the address inside the struct right after the size field.  That's the
    // location we give out when this block is allocated.
    char mem[0];

    // Pointer to the next block if this one is free.
    struct HoleStruct *next;
} Hole;

/** Pointer to the first free block. */
Hole *freeList = NULL;

// Get the first block containing at least size bytes.  Unlink it
// from the free list and return it.  Return NULL if there's no
// block big enough.
static Hole *getBlock(size_t size)
{

    // You get to write this whole function.
    Hole *pred = NULL;
    Hole *curr = freeList;

    // Look for a suitable block
    while (curr != NULL)
    {
        if (curr->size >= size)
        {
            if (pred == NULL)
                freeList = curr->next;
            else
                pred->next = curr->next;
            return curr;
        }
        pred = curr;
        curr = curr->next;
    }

    return NULL;
}

// Insert the given block back into the free list, in the right
// location to keep the free list ordered by address.  Merge the block
// with its predecessor and/or successor blocks if it's consecutive
// with them.
static void returnBlock(Hole *h)
{
    // Find the predecessor of this block in the free list.
    Hole *pred = NULL;
    for (Hole *node = freeList; node && node < h; node = node->next)
        pred = node;

    // Link this node back into the linked list.
    if (pred)
    {
        h->next = pred->next;
        pred->next = h;
    }
    else
    {
        // No predecessor, must be the first node on
        // the list.
        h->next = freeList;
        freeList = h;
    }

    // Should this node absorb its successor?
    if (h->next && h->mem + h->size == (char *)h->next)
    {
        // Yes.  Absorb the successor block into this one.  Block h
        // gets larger, by the size of its successor block and memory used
        // for its successor's size field.
        h->size += h->next->size + sizeof(size_t);

        // And the successor node no longer needs to show up as a separate
        // node in the linked list, skip over it.
        h->next = h->next->next;
    }

    // Does block h have a predecessor?
    if (pred)
    {
        // Absorb h into its predecessor if there's no gap
        // between them.

        // You get to write this part.
        if (pred->mem + pred->size == (char *)h)
        {
            // Absorb
            pred->size += h->size + sizeof(size_t);

            // Skip current
            pred->next = h->next;
        }
    }
}

void *malloc(size_t size)
{
    // We can't give out a block smaller than a pointer, otherwise,
    // we won't have enough room to link the block into the free list when
    // it's returned.
    if (size < sizeof(Hole *))
        size = sizeof(Hole *);

    // Round up to the nearest multiple of 4.  This is an effort to
    // satisfy any alignment constraints the host may have.  I'm not sure
    // if this is necessary (or sufficient) on the common platform.
    size = (size + 3) / 4 * 4;

    // Get a block from the free memory list.
    Hole *h = getBlock(size);

    // If we didn't find a large enough block, ask the OS for some more.
    if (h == NULL)
    {
        // How much do we need along with the size field itself.
        size_t req = size + sizeof(size_t);

        // Round up to a multiple of the page size, since that's what
        // we're going to get anyway.
        size_t psize = sysconf(_SC_PAGESIZE);
        req = (req + psize - 1) / psize * psize;

        // Try to request blocks contituously, starting from a fixed memory
        // address.  This will make the program's output easier to check.
        static char *nextBlock = (char *)0x600000000000;
        h = (Hole *)mmap(nextBlock, req, PROT_READ | PROT_WRITE | PROT_EXEC,
                         MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (h == MAP_FAILED)
            return NULL;
        h->size = req - sizeof(size_t);
        nextBlock += req;
    }

    // This block may be larger than what's needed.  If it's large
    // enough, split off anything extra and give it back to the free list.
    if (size + sizeof(Hole) <= h->size)
    {
        // Where would the next block start.
        Hole *h2 = (Hole *)(h->mem + size);

        // Split the available memory between the blocks.
        h2->size = h->size - size - sizeof(size_t);
        h->size = size;

        // Put the left-over block back on the free list.
        returnBlock(h2);
    }

    // Return the part of this block right after the size field.
    return &(h->mem);
}

void free(void *ptr)
{
    // The documentation says it does nothing if you free a null pointer,
    // so we had better do the same.
    if (ptr == NULL)
        return;

    // From the pointer the caller gave us, back up a few bytes to get a
    // pointer to the start of the hole it's part of.
    Hole *h = (Hole *)((char *)ptr - sizeof(size_t));

    // Put this block back on the free list.
    returnBlock(h);
}

// Dumb implementations of calloc and realloc, just based on
// malloc/free.

void *calloc(size_t count, size_t size)
{
    // How much memory do we need?
    size_t n = count * size;

    // Try to get it from malloc.
    char *p = malloc(n);

    // Zero the block before returning it.
    if (p)
        for (size_t i = 0; i < n; i++)
            p[i] = 0;

    return p;
}

// Realloc has a lot of special behaviors.  I hope I got them all.
void *realloc(void *ptr, size_t size)
{
    if (ptr == NULL)
        return malloc(size);

    if (size == 0)
    {
        free(ptr);
        return NULL;
    }

    // Try to get the new block.
    char *p2 = malloc(size);
    if (p2 == NULL)
        return NULL;

    // Figure out the size of the previous block.
    Hole *h = (Hole *)((char *)ptr - sizeof(size_t));

    // Copy everything over.
    size_t sz = h->size < size ? h->size : size;
    char *p1 = ptr;
    for (int i = 0; i < sz; i++)
        p2[i] = p1[i];

    // Free the old block and return the new one.
    free(ptr);
    return p2;
}

void reportString(const char *s)
{
    write(STDOUT_FILENO, s, strlen(s));
}

// Characters for each digit of hexadecimal.
static char hexChar[] = "0123456789ABCDEF";

void reportAddress(char *p)
{
    // We'll replace the question marks with the actual address.
    char str[20] = "0x????????????";

    // Convert the addres to a string of 12 hex digits.
    unsigned long addr = (unsigned long)p;
    for (int d = 0; d < 12; d++)
        str[d + 2] = hexChar[(addr >> (44 - d * 4)) & 0xF];

    reportString(str);
}

void reportSize(size_t s)
{
    int pos = 0;
    char str[20];

    // Convert the  to a string of hex digits.
    while (s != 0)
    {
        str[pos++] = hexChar[s & 0xF];
        s /= 16;
    }

    // Handle zeero as a special case.
    if (pos == 0)
        str[pos++] = '0';

    // Reverse the digits to get them in most-significant-first.
    for (int i = 0; i < pos - i - 1; i++)
    {
        char temp = str[i];
        str[i] = str[pos - i - 1];
        str[pos - i - 1] = temp;
    }

    // Null-terminate the string and print it.
    str[pos] = '\0';
    reportString(str);
}

void reportFreeList()
{
    for (Hole *h = freeList; h; h = h->next)
    {
        reportAddress((char *)h);
        reportString(" : ");
        reportSize(h->size);
        reportString("\n");
    }
}
