#include "uimanager.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "utils.h"

void clear_screen() {
    CLEAR_SCREEN();
}

void print_header(const char* text) {
    printf("========================================\n");
    printf("          %s\n", text);
    printf("========================================\n");
}

void print_menu_item(int index, const char* text, int selected) {
    if (selected) {
        printf("> ");
    } else {
        printf("  ");
    }
    printf("%s\n", text);
}

void print_info(const char* text) {
    printf("%s\n", text);
}

void print_hint(const char* text) {
    printf("\n%s\n", text);
}

void render_main_menu(GameState* state) {
    clear_screen();
    print_header("小黄的奇妙探险！");
    printf("\n");

    for (int i = 0; i < state->level_count; i++) {
        // 检查是否有存档
        SaveInfo info = get_save_info(state->level_names[i]);
        char menu_text[100];
        if (info.save_time > 0) {
            snprintf(menu_text, sizeof(menu_text), "开始 <%s> (上次)",
                     state->level_names[i]);
        } else {
            snprintf(menu_text, sizeof(menu_text), "开始 <%s>",
                     state->level_names[i]);
        }
        print_menu_item(i, menu_text, state->menu_selection == i);
    }

    print_menu_item(state->level_count, "退出",
                    state->menu_selection == state->level_count);

    print_hint("按 W/S 选择，按 Enter 确认");
}

void render_level_menu(GameState* state) {
    clear_screen();
    print_header("请选择关卡");
    printf("\n");

    for (int i = 0; i < state->level_count; i++) {
        print_menu_item(i, state->level_names[i],
                        state->menu_selection == i);
    }

    print_hint("按 W/S 选择，按 Enter 确认");
}

void render_mode_menu(GameState* state) {
    clear_screen();
    print_header("请选择控制模式");
    printf("\n");

    print_menu_item(0, "0: 实时模式", state->menu_selection == 0);
    print_menu_item(1, "1: 编程模式", state->menu_selection == 1);

    print_hint("按 W/S 选择，按 Enter 确认");
}

void render_load_save_menu(GameState* state, const SaveInfo* info) {
    clear_screen();
    print_header(state->level_names[state->menu_selection]);
    printf("\n");

    if (info->save_time > 0) {
        char time_str[64];
        format_time(time_str, info->save_time);

        printf("是否加载上次的进度？\n");
        printf("上次游玩时间: %s\n", time_str);
        printf("寻得的宝箱数: %d/%d\n",
               info->treasures_found, info->total_treasures);
        printf("\n");

        print_menu_item(0, "是", state->menu_selection == 0);
        print_menu_item(1, "否", state->menu_selection == 1);
    } else {
        printf("这是全新的关卡！\n\n");
        print_menu_item(0, "开始游戏", 1);
    }

    print_hint("按 W/S 选择，按 Enter 确认");
}

void render_game(GameState* state) {
    clear_screen();

    // 渲染地图
    for (int y = 0; y < state->map.height; y++) {
        for (int x = 0; x < state->map.width; x++) {
            // 检查是否是玩家位置
            if (x == state->session.player.x && y == state->session.player.y) {
                printf("Y");
            } else {
                switch (state->map.map[y][x]) {
                    case EMPTY: printf(" "); break;
                    case WALL: printf("W"); break;
                    case TRAP: printf("D"); break;
                    case TREASURE:
                        // 检查宝藏是否已收集
                    {
                        int collected = 0;
                        for (int i = 0; i < state->map.treasure_count; i++) {
                            if (state->map.treasures[i].x == x &&
                                state->map.treasures[i].y == y &&
                                state->map.treasures[i].collected) {
                                collected = 1;
                                break;
                            }
                        }
                        printf(collected ? " " : "T");
                    }
                        break;
                    default: printf(" ");
                }
            }
        }
        printf("\n");
    }

    printf("\n体力消耗: %d\n", state->session.energy_consumed);
    printf("已找到宝藏: %d/%d\n",
           state->session.treasures_found,
           state->session.total_treasures);

    if (state->session.control_mode == REALTIME_MODE) {
        printf("控制方法: WASD移动, I原地不动, Z撤销, Y恢复, Q退出\n");
    } else {
        printf("编程模式: 输入指令序列后按回车执行\n");
    }
}

void render_summary(GameState* state) {
    clear_screen();
    print_header("小黄的寻宝之旅结束");
    printf("\n");

    if (state->session.treasures_found >= state->session.total_treasures) {
        printf("恭喜你，找到了所有宝藏！\n");
    } else {
        printf("游戏结束！\n");
    }

    printf("\n行动路径: %s\n", state->session.movement_path);
    printf("消耗的体力: %d\n", state->session.energy_consumed);
    printf("找到的宝箱数量: %d\n", state->session.treasures_found);

    time_t now = time(NULL);
    double play_time = difftime(now, state->session.start_time);
    printf("游戏时间: %.0f秒\n", play_time);
}

char get_user_input() {
    return get_char_input();
}

#ifdef _WIN32
char get_char_input() {
    return _getch();
}
#else
char get_char_input() {
    struct termios oldt, newt;
    char ch;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
    return ch;
}
#endif

void get_string_input(char* buffer, int max_len) {
    fgets(buffer, max_len, stdin);
    trim_newline(buffer);
}