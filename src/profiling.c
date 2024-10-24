// profiling.c
#include "profiling.h"
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>    

void init_profiler(ProfilerData* profiler) {
    gettimeofday(&profiler->start_time, NULL);
    getrusage(RUSAGE_SELF, &profiler->last_usage);
    profiler->cpu_usage = 0.0;
    profiler->memory_usage = 0;
    profiler->sample_count = 0;
    pthread_mutex_init(&profiler->profile_mutex, NULL);
}

void calculate_metrics(ProfilerData* profiler) {
    struct rusage current_usage;
    struct timeval current_time;
    
    getrusage(RUSAGE_SELF, &current_usage);
    gettimeofday(&current_time, NULL);

    // Calculate CPU usage
    double user_time = (current_usage.ru_utime.tv_sec - profiler->last_usage.ru_utime.tv_sec) +
                      (current_usage.ru_utime.tv_usec - profiler->last_usage.ru_utime.tv_usec) / 1000000.0;
    double sys_time = (current_usage.ru_stime.tv_sec - profiler->last_usage.ru_stime.tv_sec) +
                     (current_usage.ru_stime.tv_usec - profiler->last_usage.ru_stime.tv_usec) / 1000000.0;
    double elapsed = (current_time.tv_sec - profiler->start_time.tv_sec) +
                    (current_time.tv_usec - profiler->start_time.tv_usec) / 1000000.0;

    pthread_mutex_lock(&profiler->profile_mutex);
    profiler->cpu_usage = ((user_time + sys_time) / elapsed) * 100.0;
    profiler->memory_usage = current_usage.ru_maxrss;
    profiler->sample_count++;
    pthread_mutex_unlock(&profiler->profile_mutex);

    profiler->last_usage = current_usage;
}

void log_profile_data(ProfilerData* profiler) {
    pthread_mutex_lock(&profiler->profile_mutex);
    FILE* log_file = fopen("performance_log.txt", "a");
    if (log_file) {
        fprintf(log_file, "Sample %d:\n", profiler->sample_count);
        fprintf(log_file, "CPU Usage: %.2f%%\n", profiler->cpu_usage);
        fprintf(log_file, "Memory Usage: %zu KB\n", profiler->memory_usage);
        fprintf(log_file, "------------------------\n");
        fclose(log_file);
    }
    pthread_mutex_unlock(&profiler->profile_mutex);
}

void* profiling_thread(void* arg) {
    SharedBuffer* buffer = (SharedBuffer*)arg;
    ProfilerData profiler;
    init_profiler(&profiler);

    while (!buffer->all_data_processed) {
        calculate_metrics(&profiler);
        log_profile_data(&profiler);
        usleep(1000000); // Sample every second
    }

    // Final metrics
    calculate_metrics(&profiler);
    log_profile_data(&profiler);

    pthread_mutex_destroy(&profiler.profile_mutex);
    return NULL;
}