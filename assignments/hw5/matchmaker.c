#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include "matchmaker.h"

typedef struct Player {
    char name[64];
    int skill;
    pthread_cond_t cond;
    struct Player* opponent;
    bool matched;
    bool done;
    struct Player* next;
} Player;

static int skill_range = 0;
static int skill_gap = 0;
static bool matchmaking_stopped = false;
static bool game_in_progress = false;

static pthread_mutex_t monitor_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t waiting_cond = PTHREAD_COND_INITIALIZER;

static Player* waiting_list = NULL;
static Player* playing_pair[2] = { NULL, NULL };
static int done_count = 0;

// Initializes the monitor with the given skill range and gap
void initMonitor(int range, int gap) {
    pthread_mutex_lock(&monitor_mutex);
    skill_range = range;
    skill_gap = gap;
    matchmaking_stopped = false;
    game_in_progress = false;
    waiting_list = NULL;
    done_count = 0;
    pthread_mutex_unlock(&monitor_mutex);
}

// Destroys the monitor and cleans up resources
void destroyMonitor() {
    pthread_mutex_lock(&monitor_mutex);
    Player* current = waiting_list;
    while (current) {
        Player* next = current->next;
        pthread_cond_destroy(&current->cond);
        free(current);
        current = next;
    }
    pthread_mutex_unlock(&monitor_mutex);
    pthread_mutex_destroy(&monitor_mutex);
    pthread_cond_destroy(&waiting_cond);
}

// Checks if two players are compatible based on their skill levels
static bool compatible(Player* a, Player* b) {
    int diff = abs(a->skill - b->skill);
    return (a->skill != b->skill) && (diff <= skill_gap);
}

// Finds a match for the player with the given name and skill level
bool findMatch(const char* name, int skill) {
    pthread_mutex_lock(&monitor_mutex);

    if (matchmaking_stopped) {
        pthread_mutex_unlock(&monitor_mutex);
        return false;
    }

    Player* self = malloc(sizeof(Player));
    strncpy(self->name, name, sizeof(self->name) - 1);
    self->name[sizeof(self->name) - 1] = '\0';
    self->skill = skill;
    self->matched = false;
    self->done = false;
    self->opponent = NULL;
    pthread_cond_init(&self->cond, NULL);
    self->next = NULL;

    while (!self->matched && !matchmaking_stopped) {
        Player *prev = NULL, *curr = waiting_list;
        while (curr != NULL) {
            if (!curr->matched && compatible(self, curr)) {
                self->matched = true;
                curr->matched = true;
                self->opponent = curr;
                curr->opponent = self;

                // Remove curr from waiting list
                if (prev == NULL) {
                    waiting_list = curr->next;
                } else {
                    prev->next = curr->next;
                }

                // Set current match
                playing_pair[0] = self;
                playing_pair[1] = curr;
                game_in_progress = true;

                printf("Playing: %s vs %s\n", self->name, curr->name);
                pthread_cond_signal(&curr->cond);
                break;
            }
            prev = curr;
            curr = curr->next;
        }

        if (!self->matched && !matchmaking_stopped) {
            self->next = waiting_list;
            waiting_list = self;
            pthread_cond_wait(&self->cond, &monitor_mutex);
        }
    }

    bool result = self->matched;
    pthread_mutex_unlock(&monitor_mutex);
    return result;
}

// Called by both players when they are done playing
void donePlaying() {
    pthread_mutex_lock(&monitor_mutex);
    Player* self;
    if (!playing_pair[0]->done) {
        self = playing_pair[0];
    } else {
        self = playing_pair[1];
    }
    self->done = true;
    done_count++;

    if (done_count < 2) {
        while (done_count < 2)
            pthread_cond_wait(&waiting_cond, &monitor_mutex);
    } else {
        game_in_progress = false;
    }

    pthread_cond_broadcast(&waiting_cond);
    pthread_mutex_unlock(&monitor_mutex);
}

void stopMatchmaking() {
    pthread_mutex_lock(&monitor_mutex);
    matchmaking_stopped = true;

    Player* curr = waiting_list;
    while (curr) {
        pthread_cond_signal(&curr->cond);
        curr = curr->next;
    }
    pthread_cond_broadcast(&waiting_cond);
    pthread_mutex_unlock(&monitor_mutex);
}