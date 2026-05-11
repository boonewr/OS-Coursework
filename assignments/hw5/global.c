#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
  pthread_t thread;
  char name[25];
  void *(*start)(void *);
  int dishCount;
} ChefRec;

static bool running = true;

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

// One global mutex for all chefs
static pthread_mutex_t kitchenLock = PTHREAD_MUTEX_INITIALIZER;

void *Tad(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  while (running) {
    pthread_mutex_lock(&kitchenLock);
    cook(60, rec);
    pthread_mutex_unlock(&kitchenLock);
    rest(25, rec);
  }
  return NULL;
}

void *Merry(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  while (running) {
    pthread_mutex_lock(&kitchenLock);
    cook(15, rec);
    pthread_mutex_unlock(&kitchenLock);
    rest(25, rec);
  }
  return NULL;
}

void *Charles(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  while (running) {
    pthread_mutex_lock(&kitchenLock);
    cook(90, rec);
    pthread_mutex_unlock(&kitchenLock);
    rest(25, rec);
  }
  return NULL;
}

void *Merlin(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  while (running) {
    pthread_mutex_lock(&kitchenLock);
    cook(15, rec);
    pthread_mutex_unlock(&kitchenLock);
    rest(25, rec);
  }
  return NULL;
}

void *Lyn(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  while (running) {
    pthread_mutex_lock(&kitchenLock);
    cook(75, rec);
    pthread_mutex_unlock(&kitchenLock);
    rest(25, rec);
  }
  return NULL;
}

void *Marian(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  while (running) {
    pthread_mutex_lock(&kitchenLock);
    cook(45, rec);
    pthread_mutex_unlock(&kitchenLock);
    rest(25, rec);
  }
  return NULL;
}

void *Summer(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  while (running) {
    pthread_mutex_lock(&kitchenLock);
    cook(30, rec);
    pthread_mutex_unlock(&kitchenLock);
    rest(25, rec);
  }
  return NULL;
}

void *Sammy(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  while (running) {
    pthread_mutex_lock(&kitchenLock);
    cook(60, rec);
    pthread_mutex_unlock(&kitchenLock);
    rest(25, rec);
  }
  return NULL;
}

void *Lura(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  while (running) {
    pthread_mutex_lock(&kitchenLock);
    cook(15, rec);
    pthread_mutex_unlock(&kitchenLock);
    rest(25, rec);
  }
  return NULL;
}

void *Ginny(void *arg) {
  ChefRec *rec = (ChefRec *) arg;
  while (running) {
    pthread_mutex_lock(&kitchenLock);
    cook(45, rec);
    pthread_mutex_unlock(&kitchenLock);
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
