#ifndef CONSUMER_H
#define CONSUMER_H

#include "utils.h"

typedef void (*ReportCallback)(SharedBuffer* buffer);

typedef struct {
    SharedBuffer* buffer;
    int consumer_id;
    ReportCallback callback;
} ConsumerArgs;

void* consumer_thread(void* arg);
void calculate_ppa(SharedBuffer *buffer, char* filename, char *data);
void find_max_points(SharedBuffer *buffer, char* filename, char* data);
void generate_report();
void calculate_ppa_for_tennis(SharedBuffer *buffer, char* filename, char* data);
void calculate_ppa_for_football(SharedBuffer *buffer, char* filename, char* data);

void print_ppa_and_max_points(SharedBuffer *buffer);

#endif // CONSUMER_H