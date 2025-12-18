#include "file_io.h"
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include "utils.h"
#endif

int load_map_list(GameState* state) {
    DIR* dir = opendir(MAPS_DIR);
    if (!dir) {
        printf("无法打开目录: %s\n", MAPS_DIR);
        // 创建maps目录
        create_directory(MAPS_DIR);
        return 0;
    }//打开MAPS_DIR目录，若打开失败（目录不存在），则创建目录并返回 0；

    struct dirent* entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL && count < MAX_SAVES) {
#ifdef _WIN32
        // Windows 下通过文件属性判断是否为普通文件
        char full_path[256];
        snprintf(full_path, sizeof(full_path), "%s/%s", MAPS_DIR, entry->d_name);

        DWORD attr = GetFileAttributesA(full_path);
        if (attr == INVALID_FILE_ATTRIBUTES) continue;

        // 判断是否为普通文件（非目录）
        if (attr & FILE_ATTRIBUTE_DIRECTORY) continue; // 跳过目录
#else
        // 非 Windows 系统通过 d_type 判断
        if (entry->d_type != DT_REG) continue; // 只处理普通文件
#endif

        // 检查文件扩展名是否为 .map
        char* ext = strrchr(entry->d_name, '.');
        if (ext && strcmp(ext, ".map") == 0) {
            // 移除扩展名（保留文件名部分）
            size_t name_len = strlen(entry->d_name) - 4; // 减去 .map 的长度
            strncpy(state->level_names[count], entry->d_name, name_len);
            state->level_names[count][name_len] = '\0'; // 手动添加字符串结束符
            count++;
        }
    }

    closedir(dir);
    state->level_count = count;
    return count > 0;
}
int load_map(GameState* state, const char* filename) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s.map", MAPS_DIR, filename);

    FILE* file = fopen(filepath, "r");
    if (!file) {
        return 0;
    }//打开地图文件

    // 读取地图基本信息
    fscanf(file, "%d %d", &state->map.width, &state->map.height);
    fscanf(file, "%d %d", &state->map.start_x, &state->map.start_y);

    // 读取地图数据
    state->map.treasure_count = 0;
    for (int y = 0; y < state->map.height; y++) {
        for (int x = 0; x < state->map.width; x++) {
            int tile;
            fscanf(file, "%d", &tile);
            state->map.map[y][x] = tile;

            if (tile == TREASURE) {
                state->map.treasures[state->map.treasure_count].x = x;
                state->map.treasures[state->map.treasure_count].y = y;
                state->map.treasures[state->map.treasure_count].collected = 0;
                state->map.treasure_count++;
            }
        }
    }

    strncpy(state->map.name, filename, MAX_MAP_NAME);
    fclose(file);
    return 1;
}

int save_game(const GameState* state) {
    if (state->session.current_level < 0) {
        return 0;
    }

    char savepath[256];
    snprintf(savepath, sizeof(savepath), "%s/%s.dat", SAVES_DIR,
             state->map.name);

    FILE* file = fopen(savepath, "wb");
    if (!file) {
        return 0;
    }

    // 写入保存信息
    SaveInfo info;
    strncpy(info.level_name, state->map.name, MAX_MAP_NAME);
    info.save_time = time(NULL);
    info.treasures_found = state->session.treasures_found;
    info.total_treasures = state->session.total_treasures;
    info.energy_consumed = state->session.energy_consumed;
    info.path_length = state->session.path_length;

    fwrite(&info, sizeof(SaveInfo), 1, file);
    fwrite(&state->session, sizeof(GameSession), 1, file);

    // 写入地图状态（宝藏收集状态）
    for (int i = 0; i < state->map.treasure_count; i++) {
        fwrite(&state->map.treasures[i].collected, sizeof(int), 1, file);
    }

    fclose(file);
    return 1;
}

int load_save(GameState* state, const char* level_name) {
    char savepath[256];
    snprintf(savepath, sizeof(savepath), "%s/%s.dat", SAVES_DIR, level_name);

    FILE* file = fopen(savepath, "rb");
    if (!file) {
        return 0;
    }

    // 读取保存信息
    SaveInfo info;
    fread(&info, sizeof(SaveInfo), 1, file);

    // 加载地图
    if (!load_map(state, level_name)) {
        fclose(file);
        return 0;
    }

    // 读取游戏会话
    fread(&state->session, sizeof(GameSession), 1, file);

    // 读取宝藏状态
    for (int i = 0; i < state->map.treasure_count; i++) {
        fread(&state->map.treasures[i].collected, sizeof(int), 1, file);

        // 更新地图显示
        if (state->map.treasures[i].collected) {
            state->map.map[state->map.treasures[i].y][state->map.treasures[i].x] = EMPTY;
        }
    }

    fclose(file);
    return 1;
}

SaveInfo get_save_info(const char* level_name) {
    SaveInfo info = {0};
    strncpy(info.level_name, level_name, MAX_MAP_NAME);

    char savepath[256];
    snprintf(savepath, sizeof(savepath), "%s/%s.dat", SAVES_DIR, level_name);

    FILE* file = fopen(savepath, "rb");
    if (file) {
        fread(&info, sizeof(SaveInfo), 1, file);
        fclose(file);
    }

    return info;
}

int clear_save(const char* level_name) {
    char savepath[256];
    snprintf(savepath, sizeof(savepath), "%s/%s.dat", SAVES_DIR, level_name);

    return remove(savepath) == 0;
}