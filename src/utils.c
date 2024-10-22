#include <stdlib.h>
#include "../include/utils.h"

void init_buffer(SharedBuffer* buffer, int size) {
    buffer->data = (char**)malloc(size * sizeof(char*));
    for (int i = 0; i < size; i++) {
        buffer->data[i] = (char*)malloc(1024 * sizeof(char));
    }

    buffer->players = (Player*)malloc(66000 * sizeof(Player));

    buffer->player_with_max_points_football.points = 0;
    buffer->player_with_max_points_tennis.points = 0;

    buffer->size = size;
    buffer->in = 0;
    buffer->out = 0;
    buffer->count = 0;
    buffer->player_count = 0; 
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);

    pthread_cond_init(&buffer->done_reading, NULL);

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
    
    pthread_cond_destroy(&buffer->done_reading);
}