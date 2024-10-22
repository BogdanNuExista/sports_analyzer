#ifndef CONSUMER_H
#define CONSUMER_H

#include "utils.h"

typedef void (*ReportCallback)(SharedBuffer* buffer);

typedef struct {
    SharedBuffer* buffer;
    int consumer_id;
    ReportCallback callback;
    ProfileData* profile_data;
} ConsumerArgs;

void* consumer_thread(void* arg);
void calculate_ppa(SharedBuffer *buffer, char* filename, char *data);
void find_max_points(SharedBuffer *buffer, char* filename, char* data);
void generate_report();

void* profiling_thread(void* arg);
void generate_report_callback(SharedBuffer* buffer);

#endif // CONSUMER_H