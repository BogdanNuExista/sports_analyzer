// profiling.h
#ifndef PROFILING_H
#define PROFILING_H

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "utils.h"

typedef struct {
    struct timeval start_time;
    struct rusage last_usage;
    double cpu_usage;
    size_t memory_usage;
    int sample_count;
    pthread_mutex_t profile_mutex;
} ProfilerData;

void init_profiler(ProfilerData* profiler);
void* profiling_thread(void* arg);
void log_profile_data(ProfilerData* profiler);
void calculate_metrics(ProfilerData* profiler);

#endif