#include <stdlib.h>
#include "../include/utils.h"

void init_buffer(SharedBuffer* buffer, int size) {
    buffer->data = (char**)malloc(size * sizeof(char*));
    for (int i = 0; i < size; i++) {
        buffer->data[i] = (char*)malloc(1024 * sizeof(char));
    }

    buffer->players = (Player*)malloc(66000 * sizeof(Player));

    buffer->player_with_max_points.points = 0;
    buffer->finished_reading_football = false;
    buffer->finished_reading_tennis = false;
    buffer->finished_reading_basketball = false;

    buffer->size = size;
    buffer->in = 0;
    buffer->out = 0;
    buffer->count = 0;
    buffer->player_count = 0; 
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);

    pthread_cond_init(&buffer->football_done, NULL);
    pthread_cond_init(&buffer->tennis_done, NULL);
}

void destroy_buffer(SharedBuffer* buffer) {
    for (int i = 0; i < buffer->size; i++) {
        free(buffer->data[i]);
    }
    free(buffer->data);
    free(buffer->players);
    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->not_full);
    pthread_cond_destroy(&buffer->not_empty);
    
    pthread_cond_destroy(&buffer->football_done);
    pthread_cond_destroy(&buffer->tennis_done);
}