#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <stdbool.h>

typedef struct {
    int player_id;
    char name_first[100];
    char name_last[100];
    int points; // for max points
    int w_ace, w_df, w_svpt, w_1stWon, w_2ndWon; // for wPPA = (w_ace - w_df + w_1stWon + w_2ndWon) / w_svpt
    int l_ace, l_df, l_svpt, l_1stWon, l_2ndWon; // for lPPA = (l_ace - l_df + l_1stWon + l_2ndWon) / l_svpt
    double ppa; // PPA = wPPA - lPPA
} Player;

typedef struct {
    char** data;
    int size;
    int in;
    int out;
    int count;
    char filename[1024];
    Player *players;
    Player player_with_max_points;
    int player_count;

    bool finished_reading_football;
    bool finished_reading_tennis;
    bool finished_reading_basketball;

    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
    pthread_cond_t football_done;
    pthread_cond_t tennis_done;
} SharedBuffer;

typedef struct {
    double cpu_usage;
    size_t memory_usage;
    double execution_time;
} ProfileData;

void init_buffer(SharedBuffer* buffer, int size);
void destroy_buffer(SharedBuffer* buffer);


#endif // UTILS_H