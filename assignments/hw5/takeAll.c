#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

typedef enum {
  BLENDER,
  COFFEEMAKER,
  FRYER,
  GRIDDLE,
  GRILL,
  MICROWAVE,
  MIXER,
  OVEN,
  NUM_APPLIANCES
} Appliance;

typedef struct {
  pthread_t thread;
  char name[25];
  void *(*start)(void *);
  int dishCount;
} ChefRec;

static bool running = true;
static pthread_mutex_t applianceLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t applianceCond = PTHREAD_COND_INITIALIZER;
static bool applianceInUse[NUM_APPLIANCES] = { false };

static void fail(const char *message) {
  fprintf(stderr, "%s\n", message);
  exit(1);
}

static void cook(int duration, ChefRec *chef) {
  printf("%s is cooking\n", chef->name);
  usleep(500 * ((long) rand() * duration / RAND_MAX + duration));
  chef->dishCount++;
}

static void rest(int duration, ChefRec *chef) {
  printf("%s is resting\n", chef->name);
  usleep(500 * ((long) rand() * duration / RAND_MAX + duration));
}

static void wait_and_acquire_appliances(Appliance *list, int count) {
  pthread_mutex_lock(&applianceLock);

  while (true) {
    bool allAvailable = true;
    for (int i = 0; i < count; i++) {
      if (applianceInUse[list[i]]) {
        allAvailable = false;
        break;
      }
    }

    if (allAvailable) {
      for (int i = 0; i < count; i++)
        applianceInUse[list[i]] = true;
      break;
    } else {
      pthread_cond_wait(&applianceCond, &applianceLock);
    }
  }

  pthread_mutex_unlock(&applianceLock);
}

static void release_appliances(Appliance *list, int count) {
  pthread_mutex_lock(&applianceLock);

  for (int i = 0; i < count; i++)
    applianceInUse[list[i]] = false;

  pthread_cond_broadcast(&applianceCond);
  pthread_mutex_unlock(&applianceLock);
}

// Chefs wait for all applicances to be available

void *Tad(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  Appliance needed[] = { GRIDDLE, GRILL, MICROWAVE };
  while (running) {
    wait_and_acquire_appliances(needed, 3);
    cook(60, rec);
    release_appliances(needed, 3);
    rest(25, rec);
  }
  return NULL;
}

void *Merry(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  Appliance needed[] = { MIXER, OVEN, BLENDER };
  while (running) {
    wait_and_acquire_appliances(needed, 3);
    cook(15, rec);
    release_appliances(needed, 3);
    rest(25, rec);
  }
  return NULL;
}

void *Charles(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  Appliance needed[] = { COFFEEMAKER, GRIDDLE };
  while (running) {
    wait_and_acquire_appliances(needed, 2);
    cook(90, rec);
    release_appliances(needed, 2);
    rest(25, rec);
  }
  return NULL;
}

void *Merlin(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  Appliance needed[] = { BLENDER, MIXER };
  while (running) {
    wait_and_acquire_appliances(needed, 2);
    cook(15, rec);
    release_appliances(needed, 2);
    rest(25, rec);
  }
  return NULL;
}

void *Lyn(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  Appliance needed[] = { GRIDDLE, FRYER };
  while (running) {
    wait_and_acquire_appliances(needed, 2);
    cook(75, rec);
    release_appliances(needed, 2);
    rest(25, rec);
  }
  return NULL;
}

void *Marian(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  Appliance needed[] = { COFFEEMAKER, MICROWAVE, OVEN };
  while (running) {
    wait_and_acquire_appliances(needed, 3);
    cook(45, rec);
    release_appliances(needed, 3);
    rest(25, rec);
  }
  return NULL;
}

void *Summer(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  Appliance needed[] = { FRYER, BLENDER, COFFEEMAKER };
  while (running) {
    wait_and_acquire_appliances(needed, 3);
    cook(30, rec);
    release_appliances(needed, 3);
    rest(25, rec);
  }
  return NULL;
}

void *Sammy(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  Appliance needed[] = { GRILL, FRYER };
  while (running) {
    wait_and_acquire_appliances(needed, 2);
    cook(60, rec);
    release_appliances(needed, 2);
    rest(25, rec);
  }
  return NULL;
}

void *Lura(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  Appliance needed[] = { MICROWAVE, MIXER };
  while (running) {
    wait_and_acquire_appliances(needed, 2);
    cook(15, rec);
    release_appliances(needed, 2);
    rest(25, rec);
  }
  return NULL;
}

void *Ginny(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  Appliance needed[] = { GRILL, OVEN };
  while (running) {
    wait_and_acquire_appliances(needed, 2);
    cook(45, rec);
    release_appliances(needed, 2);
    rest(25, rec);
  }
  return NULL;
}

int main(void) {
  srand(time(NULL));

  ChefRec chefList[] = {
    { .name = "Tad", .start = Tad },
    { .name = "Merry", .start = Merry },
    { .name = "Charles", .start = Charles },
    { .name = "Merlin", .start = Merlin },
    { .name = "Lyn", .start = Lyn },
    { .name = "Marian", .start = Marian },
    { .name = "Summer", .start = Summer },
    { .name = "Sammy", .start = Sammy },
    { .name = "Lura", .start = Lura },
    { .name = "Ginny", .start = Ginny },
  };

  for (int i = 0; i < sizeof(chefList) / sizeof(chefList[0]); i++) {
    if (pthread_create(&chefList[i].thread, NULL, chefList[i].start, chefList + i) != 0)
      fail("Can't create thread");
  }

  sleep(10);
  running = false;

  int total = 0;
  for (int i = 0; i < sizeof(chefList) / sizeof(chefList[0]); i++) {
    pthread_join(chefList[i].thread, NULL);
    printf("%s cooked %d dishes\n", chefList[i].name, chefList[i].dishCount);
    total += chefList[i].dishCount;
  }
  printf("Total dishes cooked: %d\n", total);
}
