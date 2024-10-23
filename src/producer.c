#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../include/producer.h"
#include "../include/utils.h"

#define MAX_PATH 1024

// Also modify process_csv_file to handle the case where consumers might have all exited
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
        
        // Check if there are still active consumers
        if (buffer->active_consumers == 0) {
            pthread_mutex_unlock(&buffer->mutex);
            fclose(file);
            return;
        }
        
        while (buffer->count == buffer->size) { 
            pthread_cond_wait(&buffer->not_full, &buffer->mutex);
            // Check again after waking up
            if (buffer->active_consumers == 0) {
                pthread_mutex_unlock(&buffer->mutex);
                fclose(file);
                return;
            }
        }
        
        strcpy(buffer->data[buffer->in], line);
        strcpy(buffer->filename, file_path);
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
    //int producer_id = args->producer_id;
    FILE *football_report, *tennis_report;

    // Phase 1: Football processing
    football_report = fopen("football_report.txt", "w");
    if (football_report == NULL) {
        printf("Error opening football report file\n");
        return NULL;
    }

    printf("Starting football phase...\n");
    read_football_players_in_shared_buffer(buffer);
    search_csv_files("data/football", buffer);
    
    // Signal end of football data
    pthread_mutex_lock(&buffer->mutex);
    buffer->phase_data_processed = true;
    pthread_cond_broadcast(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);

    // Wait for consumers to finish football processing
    pthread_mutex_lock(&buffer->completion_mutex);
    while (buffer->active_consumers > 0) {
        pthread_cond_wait(&buffer->all_done, &buffer->completion_mutex);
    }
    pthread_mutex_unlock(&buffer->completion_mutex);

    // Print football results
    fprintf(football_report, "Football Results:\n");
    fprintf(football_report, "Player with max points: %s %s, points: %d\n", 
            buffer->player_with_max_points_football.name_first,
            buffer->player_with_max_points_football.name_last,
            buffer->player_with_max_points_football.points);
    
    // Print top 10 PPA players for football
    fprintf(football_report, "\nTop 10 PPA Players:\n");
    print_top_ppa_players(buffer, football_report, true); // true for football
    fclose(football_report);

    // Phase 2: Tennis processing
    tennis_report = fopen("tennis_report.txt", "w");
    if (tennis_report == NULL) {
        printf("Error opening tennis report file\n");
        return NULL;
    }

    // Switch to tennis phase
    pthread_mutex_lock(&buffer->phase_mutex);
    buffer->current_phase = PHASE_TENNIS;
    buffer->phase_data_processed = false;
    buffer->player_count = 0; // Reset player count for tennis
    memset(buffer->players, 0, sizeof(Player) * 66000); // Reset player data
    pthread_cond_broadcast(&buffer->phase_change);
    pthread_mutex_unlock(&buffer->phase_mutex);

    printf("Starting tennis phase...\n");
    read_tennis_players_in_shared_buffer(buffer);
    search_csv_files("data/tennis", buffer);

    // Signal end of tennis data
    pthread_mutex_lock(&buffer->mutex);
    buffer->phase_data_processed = true;
    pthread_cond_broadcast(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);

    // Wait for consumers to finish tennis processing
    pthread_mutex_lock(&buffer->completion_mutex);
    while (buffer->active_consumers > 0) {
        pthread_cond_wait(&buffer->all_done, &buffer->completion_mutex);
    }
    pthread_mutex_unlock(&buffer->completion_mutex);

    // Print tennis results
    fprintf(tennis_report, "Tennis Results:\n");
    fprintf(tennis_report, "Player with max points: %s %s, points: %d\n",
            buffer->player_with_max_points_tennis.name_first,
            buffer->player_with_max_points_tennis.name_last,
            buffer->player_with_max_points_tennis.points);
    
    // Print top 10 PPA players for tennis
    fprintf(tennis_report, "\nTop 10 PPA Players:\n");
    print_top_ppa_players(buffer, tennis_report, false); // false for tennis
    fclose(tennis_report);

    // Signal completion
    pthread_mutex_lock(&buffer->phase_mutex);
    buffer->current_phase = PHASE_DONE;
    pthread_cond_broadcast(&buffer->phase_change);
    pthread_mutex_unlock(&buffer->phase_mutex);

    buffer->all_data_processed = true;
    
    free(arg);
    return NULL;
}
