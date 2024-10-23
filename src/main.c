#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/producer.h"
#include "../include/consumer.h"
#include "../include/utils.h"

#define NUM_PRODUCERS 1
#define NUM_CONSUMERS 2
#define BUFFER_SIZE 1000

int main() {
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    
    SharedBuffer buffer;
    init_buffer(&buffer, BUFFER_SIZE);

    // Create producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        ProducerArgs* args = malloc(sizeof(ProducerArgs));
        args->buffer = &buffer;
        args->producer_id = i;
        pthread_create(&producers[i], NULL, producer_thread, args);
    }

    // Create consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        ConsumerArgs* args = malloc(sizeof(ConsumerArgs));
        args->buffer = &buffer;
        args->consumer_id = i;
        pthread_create(&consumers[i], NULL, consumer_thread, args);
    }

    // Wait for producers to complete
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }

    // Wait for all consumers to finish
    pthread_mutex_lock(&buffer.completion_mutex);
    while (buffer.active_consumers > 0) {
        pthread_cond_wait(&buffer.all_done, &buffer.completion_mutex);
    }
    pthread_mutex_unlock(&buffer.completion_mutex);

    // Now safe to join consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    // Clean up
    destroy_buffer(&buffer);

    return 0;
}