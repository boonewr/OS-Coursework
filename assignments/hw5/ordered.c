#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

typedef enum
{
    BLENDER,
    GRIDDLE,
    FRYER,
    GRILL,
    MIXER,
    COFFEEMAKER,
    OVEN,
    MICROWAVE,
    NUM_APPLIANCES
} Appliance;

static pthread_mutex_t applianceLocks[NUM_APPLIANCES] = {
    [COFFEEMAKER] = PTHREAD_MUTEX_INITIALIZER,
    [BLENDER] = PTHREAD_MUTEX_INITIALIZER,
    [FRYER] = PTHREAD_MUTEX_INITIALIZER,
    [OVEN] = PTHREAD_MUTEX_INITIALIZER,
    [GRIDDLE] = PTHREAD_MUTEX_INITIALIZER,
    [MICROWAVE] = PTHREAD_MUTEX_INITIALIZER,
    [MIXER] = PTHREAD_MUTEX_INITIALIZER,
    [GRILL] = PTHREAD_MUTEX_INITIALIZER};

typedef struct
{
    pthread_t thread;
    char name[25];
    void *(*start)(void *);
    int dishCount;
} ChefRec;

static bool running = true;

static void fail(const char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

static void cook(int duration, ChefRec *chef)
{
    printf("%s is cooking\n", chef->name);
    usleep(500 * ((long)rand() * duration / RAND_MAX + duration));
    chef->dishCount++;
}

static void rest(int duration, ChefRec *chef)
{
    printf("%s is resting\n", chef->name);
    usleep(500 * ((long)rand() * duration / RAND_MAX + duration));
}

// Lock applicanes in order
static void lock_appliances(Appliance *list, int count)
{
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            if (list[j] < list[i])
            {
                Appliance temp = list[i];
                list[i] = list[j];
                list[j] = temp;
            }
        }
    }
    for (int i = 0; i < count; i++)
        pthread_mutex_lock(&applianceLocks[list[i]]);
}

// Unlock in reverse order
static void unlock_appliances(Appliance *list, int count)
{
    for (int i = count - 1; i >= 0; i--)
        pthread_mutex_unlock(&applianceLocks[list[i]]);
}

void *Tad(void *arg)
{
    ChefRec *rec = (ChefRec *)arg;
    Appliance needed[] = {GRIDDLE, GRILL, MICROWAVE};
    while (running)
    {
        lock_appliances(needed, 3);
        cook(60, rec);
        unlock_appliances(needed, 3);
        rest(25, rec);
    }
    return NULL;
}

void *Merry(void *arg)
{
    ChefRec *rec = (ChefRec *)arg;
    Appliance needed[] = {MIXER, OVEN, BLENDER};
    while (running)
    {
        lock_appliances(needed, 3);
        cook(15, rec);
        unlock_appliances(needed, 3);
        rest(25, rec);
    }
    return NULL;
}

void *Charles(void *arg)
{
    ChefRec *rec = (ChefRec *)arg;
    Appliance needed[] = {COFFEEMAKER, GRIDDLE};
    while (running)
    {
        lock_appliances(needed, 2);
        cook(90, rec);
        unlock_appliances(needed, 2);
        rest(25, rec);
    }
    return NULL;
}

void *Merlin(void *arg)
{
    ChefRec *rec = (ChefRec *)arg;
    Appliance needed[] = {BLENDER, MIXER};
    while (running)
    {
        lock_appliances(needed, 2);
        cook(15, rec);
        unlock_appliances(needed, 2);
        rest(25, rec);
    }
    return NULL;
}

void *Lyn(void *arg)
{
    ChefRec *rec = (ChefRec *)arg;
    Appliance needed[] = {GRIDDLE, FRYER};
    while (running)
    {
        lock_appliances(needed, 2);
        cook(75, rec);
        unlock_appliances(needed, 2);
        rest(25, rec);
    }
    return NULL;
}

void *Marian(void *arg)
{
    ChefRec *rec = (ChefRec *)arg;
    Appliance needed[] = {COFFEEMAKER, MICROWAVE, OVEN};
    while (running)
    {
        lock_appliances(needed, 3);
        cook(45, rec);
        unlock_appliances(needed, 3);
        rest(25, rec);
    }
    return NULL;
}

void *Summer(void *arg)
{
    ChefRec *rec = (ChefRec *)arg;
    Appliance needed[] = {FRYER, BLENDER, COFFEEMAKER};
    while (running)
    {
        lock_appliances(needed, 3);
        cook(30, rec);
        unlock_appliances(needed, 3);
        rest(25, rec);
    }
    return NULL;
}

void *Sammy(void *arg)
{
    ChefRec *rec = (ChefRec *)arg;
    Appliance needed[] = {GRILL, FRYER};
    while (running)
    {
        lock_appliances(needed, 2);
        cook(60, rec);
        unlock_appliances(needed, 2);
        rest(25, rec);
    }
    return NULL;
}

void *Lura(void *arg)
{
    ChefRec *rec = (ChefRec *)arg;
    Appliance needed[] = {MICROWAVE, MIXER};
    while (running)
    {
        lock_appliances(needed, 2);
        cook(15, rec);
        unlock_appliances(needed, 2);
        rest(25, rec);
    }
    return NULL;
}

void *Ginny(void *arg)
{
    ChefRec *rec = (ChefRec *)arg;
    Appliance needed[] = {GRILL, OVEN};
    while (running)
    {
        lock_appliances(needed, 2);
        cook(45, rec);
        unlock_appliances(needed, 2);
        rest(25, rec);
    }
    return NULL;
}

int main(void)
{
    srand(time(NULL));

    ChefRec chefList[] = {
        {.name = "Tad", .start = Tad},
        {.name = "Merry", .start = Merry},
        {.name = "Charles", .start = Charles},
        {.name = "Merlin", .start = Merlin},
        {.name = "Lyn", .start = Lyn},
        {.name = "Marian", .start = Marian},
        {.name = "Summer", .start = Summer},
        {.name = "Sammy", .start = Sammy},
        {.name = "Lura", .start = Lura},
        {.name = "Ginny", .start = Ginny},
    };

    for (int i = 0; i < sizeof(chefList) / sizeof(chefList[0]); i++)
    {
        if (pthread_create(&chefList[i].thread, NULL, chefList[i].start, chefList + i) != 0)
            fail("Can't create thread");
    }

    sleep(10);
    running = false;

    int total = 0;
    for (int i = 0; i < sizeof(chefList) / sizeof(chefList[0]); i++)
    {
        pthread_join(chefList[i].thread, NULL);
        printf("%s cooked %d dishes\n", chefList[i].name, chefList[i].dishCount);
        total += chefList[i].dishCount;
    }
    printf("Total dishes cooked: %d\n", total);
}
