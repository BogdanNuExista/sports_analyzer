#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum {
    PHASE_FOOTBALL,
    PHASE_TENNIS,
    PHASE_DONE
} ProcessingPhase;

typedef struct {
    int player_id;
    char name_first[100];
    char name_last[100];
    int points; // for max points
    int w_ace, w_df, w_svpt, w_1stWon, w_2ndWon; // for wPPA = (w_ace - w_df + w_1stWon + w_2ndWon) / w_svpt
    int l_ace, l_df, l_svpt, l_1stWon, l_2ndWon; // for lPPA = (l_ace - l_df + l_1stWon + l_2ndWon) / l_svpt
    double ppa; // PPA = wPPA - lPPA
} Player;

typedef void (*TaskCompletionCallback)(void* result);

typedef struct {
    char** data;
    int size;
    int in;
    int out;
    int count;
    char filename[1024];
    Player *players;
    Player player_with_max_points_tennis;
    Player player_with_max_points_football;
    int player_count;

    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
    pthread_cond_t done_reading;

    ProcessingPhase current_phase;
    bool phase_data_processed;
    pthread_mutex_t phase_mutex;
    pthread_cond_t phase_change;

    bool all_data_processed;
    int active_consumers;
    pthread_mutex_t completion_mutex;
    pthread_cond_t all_done;
} SharedBuffer;

typedef struct {
    double start_time;
    double cpu_usage;
    size_t memory_usage;
    pthread_mutex_t profile_mutex;
} ProfileInfo;

typedef struct {
    SharedBuffer* buffer;
    int thread_id;
    TaskCompletionCallback callback;
    ProfileInfo* profile_info;
} ThreadArgs;

void init_buffer(SharedBuffer* buffer, int size);
void destroy_buffer(SharedBuffer* buffer);
void print_top_ppa_players(SharedBuffer* buffer, FILE* file, bool is_football);

#endif // UTILS_H