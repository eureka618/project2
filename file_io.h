#ifndef FILE_IO_H
#define FILE_IO_H

#include "gamestate.h"

#define MAPS_DIR "maps"//地图文件夹
#define SAVES_DIR "saves"//// 存档文件夹

typedef struct {
    char level_name[MAX_MAP_NAME];
    time_t save_time;
    int treasures_found;
    int total_treasures;
    int energy_consumed;
    int path_length;
} SaveInfo;

int load_map_list(GameState* state);//加载可用地图列表
int load_map(GameState* state, const char* filename);//加载特定地图文件
int save_game(const GameState* state);
int load_save(GameState* state, const char* level_name);//加载存档
SaveInfo get_save_info(const char* level_name);
int clear_save(const char* level_name);

#endif