#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../include/producer.h"
#include "../include/utils.h"

#define MAX_PATH 1024

void process_csv_file(const char* file_path, SharedBuffer* buffer) {
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", file_path);
        return;
    }

    char line[1024];
    fgets(line, sizeof(line), file); // Skip header
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        
        // Add data to buffer
        pthread_mutex_lock(&buffer->mutex);
        while (buffer->count == buffer->size) { 
            pthread_cond_wait(&buffer->not_full, &buffer->mutex);
        }
        
        strcpy(buffer->data[buffer->in], line);
        buffer->in = (buffer->in + 1) % buffer->size;
        buffer->count++;
        
        pthread_cond_signal(&buffer->not_empty);
        pthread_mutex_unlock(&buffer->mutex);
    }

    printf("Finished reading file: %s\n", file_path);
    fclose(file);
}

void search_csv_files(const char* dir_path, SharedBuffer* buffer) {
    DIR* dir;
    struct dirent* entry;
    char path[MAX_PATH];

    if ((dir = opendir(dir_path)) == NULL) {
        perror("opendir() error");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        if(entry->d_type == DT_DIR) {
            search_csv_files(path, buffer);
        } else {
            size_t len = strlen(entry->d_name);
            if (len > 4 && strcmp(entry->d_name + len - 4, ".csv") == 0) {

                strcpy(buffer->filename, path);
                
                process_csv_file(path, buffer);
            }
        }
    }

    closedir(dir);
}

void read_football_players_in_shared_buffer(SharedBuffer *buffer)
{
    FILE* file = fopen("data/football/atp_players.csv", "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", "data/football/atp_players.csv");
        return;
    }
    char line[1024];
    fgets(line, sizeof(line), file); // Skip header
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        
        // Add data to buffer
        pthread_mutex_lock(&buffer->mutex);
        while (buffer->count == buffer->size) { 
            pthread_cond_wait(&buffer->not_full, &buffer->mutex);
        }
        
        char *p=strtok(line, ",");
        int i=0;
        while(p!=NULL)
        {
            switch(i)
            {
                case 0:
                    buffer->players[buffer->player_count].player_id = atoi(p);
                    break;
                case 1:
                    strcpy(buffer->players[buffer->player_count].name_first, p);
                    break;
                case 2:
                    strcpy(buffer->players[buffer->player_count].name_last, p);
                    break;
            }
            p=strtok(NULL, ",");
            i++;
        }
        buffer->player_count++;
        
        pthread_cond_signal(&buffer->not_empty);
        pthread_mutex_unlock(&buffer->mutex);
    }

    printf("Finished adding football players to buffer, size %d\n", buffer->player_count);
}

void read_tennis_players_in_shared_buffer(SharedBuffer *buffer)
{
    FILE* file = fopen("data/tennis/atp_players.csv", "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", "data/tennis/atp_players.csv");
        return;
    }
    char line[1024];
    fgets(line, sizeof(line), file); // Skip header
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        
        // Add data to buffer
        pthread_mutex_lock(&buffer->mutex);
        while (buffer->count == buffer->size) { 
            pthread_cond_wait(&buffer->not_full, &buffer->mutex);
        }
        
        char *p=strtok(line, ",");
        int i=0;
        while(p!=NULL)
        {
            switch(i)
            {
                case 0:
                    buffer->players[buffer->player_count].player_id = atoi(p);
                    break;
                case 1:
                    strcpy(buffer->players[buffer->player_count].name_first, p);
                    break;
                case 2:
                    strcpy(buffer->players[buffer->player_count].name_last, p);
                    break;
            }
            p=strtok(NULL, ",");
            i++;
        }
        buffer->player_count++;
        
        pthread_cond_signal(&buffer->not_empty);
        pthread_mutex_unlock(&buffer->mutex);
    }

    printf("Finished adding tennis players to buffer, size %d\n", buffer->player_count);
}

// TODO: Implement the rest for basketball

void* producer_thread(void* arg) {
    ProducerArgs* args = (ProducerArgs*)arg;
    SharedBuffer* buffer = args->buffer;
    int producer_id = args->producer_id;
    char data_dir[MAX_PATH] = "data/football";

    read_football_players_in_shared_buffer(buffer);

    search_csv_files(data_dir, buffer);

    printf("Producer %d finished processing all football CSV files.\n", producer_id);
    buffer->finished_reading_football = true;

    buffer->player_count = 0;
    read_tennis_players_in_shared_buffer(buffer);
    strcpy(data_dir, "data/tennis");
    search_csv_files(data_dir, buffer);

    printf("Producer %d finished processing all tennis CSV files.\n", producer_id);
    buffer->finished_reading_tennis = true;

    
    printf("Producer %d finished processing all CSV files.\n", producer_id);

    free(arg);
    return NULL;
}