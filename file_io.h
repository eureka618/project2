#ifndef FILE_IO_H
#define FILE_IO_H

#include "gamestate.h"

#define MAPS_DIR "maps"
#define SAVES_DIR "saves"

typedef struct {
    char level_name[MAX_MAP_NAME];
    time_t save_time;
    int treasures_found;
    int total_treasures;
    int energy_consumed;
    int path_length;
} SaveInfo;

int load_map_list(GameState* state);
int load_map(GameState* state, const char* filename);
int save_game(const GameState* state);
int load_save(GameState* state, const char* level_name);
SaveInfo get_save_info(const char* level_name);
int clear_save(const char* level_name);

#endif