#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/producer.h"
#include "../include/consumer.h"
#include "../include/utils.h"

#define NUM_PRODUCERS 1
#define NUM_CONSUMERS 3
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
    for (int i = 0; i < NUM_CONSUMERS-1; i++) {
        ConsumerArgs* args = malloc(sizeof(ConsumerArgs));
        args->buffer = &buffer;
        args->consumer_id = i;
        pthread_create(&consumers[i], NULL, consumer_thread, args);
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    // Now the 3rd thread for the report

    // ConsumerArgs* args = malloc(sizeof(ConsumerArgs));
    // args->buffer = &buffer;
    // args->consumer_id = 2;
    // pthread_create(&consumers[2], NULL, consumer_thread, args);

    // pthread_join(consumers[2], NULL);

    // Clean up
    destroy_buffer(&buffer);

    return 0;
}