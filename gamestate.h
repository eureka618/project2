#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <time.h>

#define MAX_WIDTH 100
#define MAX_HEIGHT 100
#define MAX_PATH 10000
#define MAX_SAVES 10
#define MAX_MAP_NAME 50

typedef enum {
    EMPTY = 0,
    WALL = 1,
    TRAP = 2,
    TREASURE = 3
} TileType;

typedef enum {
    REALTIME_MODE = 0,
    PROGRAMMING_MODE = 1
} ControlMode;

typedef enum {
    MENU_MAIN,
    MENU_LEVEL,
    MENU_LOAD_SAVE,
    MENU_MODE,
    GAME_PLAYING,
    GAME_SUMMARY,
} GameScreen;

typedef struct {
    int x;
    int y;
    int energy;
    int step_cost;
} Player;

typedef struct {
    int x;
    int y;
    int collected;
} Treasure;

typedef struct {
    char name[MAX_MAP_NAME];
    int width;
    int height;
    int start_x;
    int start_y;
    int treasure_count;
    int map[MAX_HEIGHT][MAX_WIDTH];
    Treasure treasures[MAX_WIDTH * MAX_HEIGHT / 10];
} GameMap;

typedef struct {
    Player player;
    int treasures_found;
    int total_treasures;
    int energy_consumed;
    char movement_path[MAX_PATH];
    int path_length;
    ControlMode control_mode;
    time_t start_time;
    time_t save_time;
    int current_level;
    int game_over;
} GameSession;

typedef struct {
    GameMap map;
    GameSession session;
    GameScreen current_screen;
    int menu_selection;
    int level_count;
    int mode_selection;
    int load_save_selection;
    char level_names[MAX_SAVES][MAX_MAP_NAME];
    char last_played_level[MAX_MAP_NAME];
} GameState;

#endif