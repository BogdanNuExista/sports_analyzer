#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/consumer.h"
#include "../include/utils.h"

void* consumer_thread(void* arg) {
    ConsumerArgs* args = (ConsumerArgs*)arg;
    SharedBuffer* buffer = args->buffer;
    int consumer_id = args->consumer_id;

    while (1) {
        char data[1024];
        char filename[1024];
        
        pthread_mutex_lock(&buffer->mutex);
        while (buffer->count == 0) {
            pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
        }
        
        strcpy(data, buffer->data[buffer->out]);
        strcpy(filename, buffer->filename);
        buffer->out = (buffer->out + 1) % buffer->size;
        buffer->count--;
        
        pthread_cond_signal(&buffer->not_full);
        pthread_mutex_unlock(&buffer->mutex);

        // Process data based on consumer_id
        switch (consumer_id) {
            case 0:
                calculate_ppa(buffer, filename, data);
                break;
            case 1:
                find_max_points(buffer, filename, data);
                break;
            case 2:
                generate_report(data);
                break;
        }
    }

    free(arg);
    return NULL;
}

//////////////////////////////////////////////////////////// HELPER FUNCTIONS ////////////////////////////////////////////////////////////

int find_player_by_name(Player *players, int player_count, char* name) {
    for (int i = 0; i < player_count; i++) {
        char full_name[200];
        sprintf(full_name, "%s %s", players[i].name_first, players[i].name_last);
        if (strcmp(players[i].name_first, name) == 0) {
            return i;
        }
    }
    return -1;
}

int find_player_by_id(Player *players, int player_count, int id) {
    for (int i = 0; i < player_count; i++) {
        if (players[i].player_id == id) {
            return i;
        }
    }
    return -1;
}


// tourney_id,tourney_name,surface,draw_size,tourney_level,tourney_date,match_num,winner_id,winner_seed,winner_entry,
// winner_name,winner_hand,winner_ht,winner_ioc,winner_age,loser_id,loser_seed,loser_entry,loser_name,loser_hand,loser_ht,
// loser_ioc,loser_age,score,best_of,round,minutes,w_ace,w_df,w_svpt,w_1stIn,w_1stWon,w_2ndWon,w_SvGms,w_bpSaved,w_bpFaced,
// l_ace,l_df,l_svpt,l_1stIn,l_1stWon,l_2ndWon,l_SvGms,l_bpSaved,l_bpFaced,winner_rank,winner_rank_points,loser_rank,loser_rank_points

// the problem is that when adding too much logic, the recieved data is smaller for some reason
void calculate_ppa_for_football(SharedBuffer *buffer, char* filename, char* data) {

    if(strncmp(filename, "data/football/atp_players", 25) == 0) { // already processed players in producer
        return;
    }

    if(strncmp(filename, "data/football/rankings", 22) == 0) { // this is for max points, not PPA
        return;
    }

    if(strncmp(filename, "data/football", 13) != 0) { // accept football data only first
        return;
    }

    int field_count = 0;
    int w_ace = 0, w_df = 0, w_svpt = 0, w_1stWon = 0, w_2ndWon = 0;
    int l_ace = 0, l_df = 0, l_svpt = 0, l_1stWon = 0, l_2ndWon = 0;
    char winner_name[100] = {0};
    char loser_name[100] = {0};
    int winner_id = 0, loser_id = 0;

    char* token;
    char* rest = data;
    char* end;

    while (1) {
        // Find the next comma or end of string
        end = strchr(rest, ',');
        if (end == NULL) {
            end = rest + strlen(rest);
        }

        // Create a temporary null-terminated token
        char temp = *end;
        *end = '\0';
        token = rest;

        // Process the token
        switch (field_count) {
            case 7: winner_id = atoi(token); break;
            case 10: strncpy(winner_name, token, sizeof(winner_name) - 1); break;
            case 15: loser_id = atoi(token); break;
            case 18: strncpy(loser_name, token, sizeof(loser_name) - 1); break;
            case 27: w_ace = atoi(token); break;
            case 28: w_df = atoi(token); break;
            case 29: w_svpt = atoi(token); break;
            case 31: w_1stWon = atoi(token); break;
            case 32: w_2ndWon = atoi(token); break;
            case 33: l_ace = atoi(token); break;
            case 34: l_df = atoi(token); break;
            case 35: l_svpt = atoi(token); break;
            case 37: l_1stWon = atoi(token); break;
            case 38: l_2ndWon = atoi(token); break;
        }

        field_count++;

        // Restore the original character and move to the next field
        *end = temp;
        if (*end == '\0') {
            break;
        }
        rest = end + 1;
    }

    // Remove trailing newline characters from names
    winner_name[strcspn(winner_name, "\r\n")] = 0;
    loser_name[strcspn(loser_name, "\r\n")] = 0;

    //printf("Winner: %s, Loser: %s - w_ace: %d, w_df: %d, w_svpt: %d, w_1stWon: %d, w_2ndWon: %d, l_ace: %d, l_df: %d, l_svpt: %d, l_1stWon: %d, l_2ndWon: %d\n", winner_name, loser_name, w_ace, w_df, w_svpt, w_1stWon, w_2ndWon, l_ace, l_df, l_svpt, l_1stWon, l_2ndWon);

    if(w_svpt == 0 || l_svpt == 0) {
        return;
    }

    double wPPA = (double)(w_ace - w_df + w_1stWon + w_2ndWon) / w_svpt;
    double lPPA = (double)(l_ace - l_df + l_1stWon + l_2ndWon) / l_svpt;

    int find_winner = find_player_by_id(buffer->players, buffer->player_count, winner_id);
    int find_loser = find_player_by_id(buffer->players, buffer->player_count, loser_id);

    if (find_winner != -1 && find_loser != -1) {
        buffer->players[find_winner].ppa += wPPA - lPPA;
        buffer->players[find_loser].ppa += lPPA - wPPA;
        //printf("OMG PLAYER FOUND IN BUFFER\n");
    } else {
        printf("Warning: Player not found. Winner: %s, Loser: %s\n", winner_name, loser_name);
    }
}

void calculate_ppa_for_tennis(SharedBuffer *buffer, char* filename, char* data) {

    if(strncmp(filename, "data/tennis/atp_players", 25) == 0) { // already processed players in producer
        return;
    }

    if(strncmp(filename, "data/tennis/rankings", 22) == 0) { // this is for max points, not PPA
        return;
    }

    if(strncmp(filename, "data/tennis", 11) != 0) { // accept tennis data only first
        return;
    }

    int field_count = 0;
    int w_ace = 0, w_df = 0, w_svpt = 0, w_1stWon = 0, w_2ndWon = 0;
    int l_ace = 0, l_df = 0, l_svpt = 0, l_1stWon = 0, l_2ndWon = 0;
    char winner_name[100] = {0};
    char loser_name[100] = {0};
    int winner_id = 0, loser_id = 0;

    char* token;
    char* rest = data;
    char* end;

    while (1) {
        // Find the next comma or end of string
        end = strchr(rest, ',');
        if (end == NULL) {
            end = rest + strlen(rest);
        }

        // Create a temporary null-terminated token
        char temp = *end;
        *end = '\0';
        token = rest;

        // Process the token
        switch (field_count) {
            case 7: winner_id = atoi(token); break;
            case 10: strncpy(winner_name, token, sizeof(winner_name) - 1); break;
            case 15: loser_id = atoi(token); break;
            case 18: strncpy(loser_name, token, sizeof(loser_name) - 1); break;
            case 27: w_ace = atoi(token); break;
            case 28: w_df = atoi(token); break;
            case 29: w_svpt = atoi(token); break;
            case 31: w_1stWon = atoi(token); break;
            case 32: w_2ndWon = atoi(token); break;
            case 33: l_ace = atoi(token); break;
            case 34: l_df = atoi(token); break;
            case 35: l_svpt = atoi(token); break;
            case 37: l_1stWon = atoi(token); break;
            case 38: l_2ndWon = atoi(token); break;
        }

        field_count++;

        // Restore the original character and move to the next field
        *end = temp;
        if (*end == '\0') {
            break;
        }
        rest = end + 1;
    }

    // Remove trailing newline characters from names
    winner_name[strcspn(winner_name, "\r\n")] = 0;
    loser_name[strcspn(loser_name, "\r\n")] = 0;

    //printf("Winner: %s, Loser: %s - w_ace: %d, w_df: %d, w_svpt: %d, w_1stWon: %d, w_2ndWon: %d, l_ace: %d, l_df: %d, l_svpt: %d, l_1stWon: %d, l_2ndWon: %d\n", winner_name, loser_name, w_ace, w_df, w_svpt, w_1stWon, w_2ndWon, l_ace, l_df, l_svpt, l_1stWon, l_2ndWon);

    if(w_svpt == 0 || l_svpt == 0) {
        return;
    }

    double wPPA = (double)(w_ace - w_df + w_1stWon + w_2ndWon) / w_svpt;
    double lPPA = (double)(l_ace - l_df + l_1stWon + l_2ndWon) / l_svpt;

    int find_winner = find_player_by_id(buffer->players, buffer->player_count, winner_id);
    int find_loser = find_player_by_id(buffer->players, buffer->player_count, loser_id);

    if (find_winner != -1 && find_loser != -1) {
        buffer->players[find_winner].ppa += wPPA - lPPA;
        buffer->players[find_loser].ppa += lPPA - wPPA;
        //printf("OMG PLAYER FOUND IN BUFFER\n");
    } else {
        printf("Warning: Player not found. Winner: %s, Loser: %s\n", winner_name, loser_name);
    }
}

void calculate_ppa_for_basket()
{
    //TODO
}

void calculate_max_points_for_football(SharedBuffer *buffer, char *filename, char *data)
{
    if(strncmp(filename, "data/football/atp_rankings", 26) != 0) { // already processed players in producer
        return;
    }

    char *p=strtok(data, ",");
    int i=0;
    int p_id = 0;
    int p_points = 0;
    while(p)
    {
        switch(i)
        {
            case 2: p_id = atoi(p); break;
            case 3: p_points = atoi(p); break;
        }
        p=strtok(NULL, ",");
        i++;
    }

    int find_player = find_player_by_id(buffer->players, buffer->player_count, p_id);
    if (find_player != -1) {
        buffer->players[find_player].points += p_points;
        if(buffer->players[find_player].points > buffer->player_with_max_points.points) {
            buffer->player_with_max_points = buffer->players[find_player];
        }
    } else {
        printf("Warning: Player not found when calc max points for football. Player ID: %d\n", p_id);
    }
}

void calculate_max_points_for_tennis(SharedBuffer *buffer, char *filename, char *data)
{
    if(strncmp(filename, "data/tennis/atp_rankings", 26) != 0) { // already processed players in producer
        return;
    }

    char *p=strtok(data, ",");
    int i=0;
    int p_id = 0;
    int p_points = 0;
    while(p)
    {
        switch(i)
        {
            case 2: p_id = atoi(p); break;
            case 3: p_points = atoi(p); break;
        }
        p=strtok(NULL, ",");
        i++;
    }

    int find_player = find_player_by_id(buffer->players, buffer->player_count, p_id);
    if (find_player != -1) {
        buffer->players[find_player].points += p_points;
        if(buffer->players[find_player].points > buffer->player_with_max_points.points) {
            buffer->player_with_max_points = buffer->players[find_player];
        }
    } else {
        printf("Warning: Player not found when calc max points for tennis. Player ID: %d\n", p_id);
    }
}



//////////////////////////////////////////////////////////// HELPER FUNCTIONS ////////////////////////////////////////////////////////////

void calculate_ppa(SharedBuffer *buffer, char* filename, char* data) {
    // printf("Calculating PPA for: %s\n", data);
    calculate_ppa_for_football(buffer, filename, data);
    calculate_ppa_for_tennis(buffer, filename, data);
}

void find_max_points(SharedBuffer *buffer, char* filename, char* data) {
    // printf("Finding max points for: %s\n", data);
    calculate_max_points_for_football(buffer, filename, data);
    if(buffer->finished_reading_football) {
        printf("Player with max points at football: %s %s, Points: %d\n", buffer->player_with_max_points.name_first, buffer->player_with_max_points.name_last, buffer->player_with_max_points.points);
        buffer->finished_reading_football = false;
    }
    calculate_max_points_for_tennis(buffer, filename, data);
    if(buffer->finished_reading_tennis) {
        printf("Player with max points at tennis: %s %s, Points: %d\n", buffer->player_with_max_points.name_first, buffer->player_with_max_points.name_last, buffer->player_with_max_points.points);
        buffer->finished_reading_tennis = false;
    }
}

void generate_report() {

    /*
    printf("Player Production Average (PPA) Report:\n");
    printf("---------------------------------------\n");
    for (int i = 0; i < num_players; i++) {
        if (player_stats[i].games_played > 0) {
            double avg_ppa = player_stats[i].total_ppa / player_stats[i].games_played;
            printf("Player: %s\n", player_stats[i].name);
            printf("Average PPA: %.4f\n", avg_ppa);
            printf("Max Points in a Game: %d\n", player_stats[i].max_points);
            printf("---------------------------------------\n");
        }
    }
    */
}