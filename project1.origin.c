#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#define CLEAR_SCREEN() system("cls")
#else
#include <termios.h>
#include <unistd.h>
#define CLEAR_SCREEN() system("clear")

// 模拟Windows的getch函数
int getch(void) {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

// 常量定义
#define MAX_WIDTH 50
#define MAX_HEIGHT 50
#define MAX_PATH 1000
#define MAX_TREASURES 10

// 地块类型
typedef enum {
    EMPTY = 0,
    WALL = 1,
    TRAP = 2,
    TREASURE = 3
} TileType;

// 控制模式
typedef enum {
    REALTIME_MODE = 0,
    PROGRAMMING_MODE = 1
} ControlMode;

// 游戏状态
typedef struct {
    int map[MAX_HEIGHT][MAX_WIDTH];
    int width, height;
    int player_x, player_y;
    int start_x, start_y;
    int treasures[MAX_TREASURES][2];
    int treasure_count;
    int found_treasures;
    int energy_consumed;
    int step_cost;
    char path[MAX_PATH];
    int path_length;
    ControlMode control_mode;
    int game_over;
} GameState;

// 函数声明
void initialize_game(GameState *game);
void load_level(GameState *game, int level);
void render_welcome_screen();
void render_mode_selection();
void render_game(GameState *game);
void move_player(GameState *game, char direction);
int is_valid_move(GameState *game, int x, int y);
void check_treasure(GameState *game);
void game_summary(GameState *game, const char *reason);
void process_programming_mode(GameState *game);

// 初始化游戏
void initialize_game(GameState *game) {
    game->player_x = game->start_x;
    game->player_y = game->start_y;
    game->found_treasures = 0;
    game->energy_consumed = 0;
    game->step_cost = 1;
    game->path_length = 0;
    game->game_over = 0;
    for (int i = 0; i < MAX_PATH; i++) {
        game->path[i] = '\0';
    }
}

// 加载关卡数据
void load_level(GameState *game, int level) {
    // 这里简化了地图数据，实际应该从文件或硬编码读取
    if (level == 1) {
        // 第一关简单地图
        game->width = 10;
        game->height = 8;
        game->start_x = 1;
        game->start_y = 1;
        game->treasure_count = 1;

        // 初始化地图 - 边界为墙
        for (int i = 0; i < game->height; i++) {
            for (int j = 0; j < game->width; j++) {
                if (i == 0 || i == game->height-1 || j == 0 || j == game->width-1) {
                    game->map[i][j] = WALL;
                } else {
                    game->map[i][j] = EMPTY;
                }
            }
        }

        // 设置宝藏
        game->map[6][8] = TREASURE;
        game->treasures[0][0] = 8;
        game->treasures[0][1] = 6;

        // 设置一些墙
        game->map[2][3] = WALL;
        game->map[3][3] = WALL;
        game->map[4][3] = WALL;
        game->map[5][6] = WALL;

    } else if (level == 2) {
        // 第二关更复杂的地图
        game->width = 12;
        game->height = 10;
        game->start_x = 1;
        game->start_y = 1;
        game->treasure_count = 2;

        // 初始化地图
        for (int i = 0; i < game->height; i++) {
            for (int j = 0; j < game->width; j++) {
                if (i == 0 || i == game->height-1 || j == 0 || j == game->width-1) {
                    game->map[i][j] = WALL;
                } else {
                    game->map[i][j] = EMPTY;
                }
            }
        }

        // 设置宝藏
        game->map[3][10] = TREASURE;
        game->map[8][5] = TREASURE;
        game->treasures[0][0] = 10;
        game->treasures[0][1] = 3;
        game->treasures[1][0] = 5;
        game->treasures[1][1] = 8;

        // 设置墙和陷阱
        game->map[2][4] = WALL;
        game->map[3][4] = WALL;
        game->map[4][4] = WALL;
        game->map[5][7] = TRAP;
        game->map[6][7] = TRAP;
        game->map[7][3] = WALL;
        game->map[7][8] = TRAP;
    }
}

// 渲染欢迎界面
void render_welcome_screen() {
    CLEAR_SCREEN();
    printf("========================================\n");
    printf("          小黄的奇妙探险！            \n");
    printf("========================================\n");
    printf("\n");
    printf("> 开始第一关\n");
    printf("  开始第二关\n");
    printf("  退出\n");
    printf("\n");
    printf("控制方法：按 W 向上移动，按 S 向下移动，按 <Enter> 选择\n");
}

// 渲染模式选择界面
void render_mode_selection() {
    CLEAR_SCREEN();
    printf("请选择控制模式：\n");
    printf("> 0: 实时模式\n");
    printf("  1: 编程模式\n");
    printf("\n");
    printf("控制方法：按 W 向上移动，按 S 向下移动，按 <Enter> 选择\n");
}

// 渲染游戏界面
void render_game(GameState *game) {
    CLEAR_SCREEN();

    // 打印地图
    for (int i = 0; i < game->height; i++) {
        for (int j = 0; j < game->width; j++) {
            if (i == game->player_y && j == game->player_x) {
                printf("Y"); // 小黄
            } else {
                switch (game->map[i][j]) {
                    case EMPTY: printf(" "); break;
                    case WALL: printf("W"); break;
                    case TRAP: printf("D"); break;
                    case TREASURE:
                        // 检查宝藏是否已经被找到
                        int found = 0;
                        for (int t = 0; t < game->treasure_count; t++) {
                            if (game->treasures[t][0] == j && game->treasures[t][1] == i) {
                                found = 1;
                                break;
                            }
                        }
                        printf(found ? "T" : " ");
                        break;
                }
            }
        }
        printf("\n");
    }

    printf("\n体力消耗：%d\n", game->energy_consumed);
    printf("控制方法：按W向上移动，按S向下移动，按A向左移动，按D向右移动，按I原地不动，按Q结束冒险\n");
    if (game->control_mode == PROGRAMMING_MODE) {
        printf("编程模式：输入移动序列后按回车执行\n");
    }
}

// 检查移动是否有效
int is_valid_move(GameState *game, int x, int y) {
    if (x < 0 || x >= game->width || y < 0 || y >= game->height) {
        return 0;
    }
    return game->map[y][x] != WALL;
}

// 移动玩家
void move_player(GameState *game, char direction) {
    int new_x = game->player_x;
    int new_y = game->player_y;

    switch (toupper(direction)) {
        case 'W': new_y--; break; // 上
        case 'S': new_y++; break; // 下
        case 'A': new_x--; break; // 左
        case 'D': new_x++; break; // 右
        case 'I': break;          // 原地不动
        case 'Q':
            game->game_over = 1;
            game_summary(game, "用户退出");
            return;
        default:
            if (game->control_mode == REALTIME_MODE) {
                printf("输入错误，请重新输入\n");
            } else {
                game->game_over = 1;
                game_summary(game, "编程模式输入错误");
            }
            return;
    }

    // 记录移动
    if (direction != 'I' && direction != 'Q') {
        game->path[game->path_length++] = toupper(direction);
    }

    // 检查移动是否有效
    if (is_valid_move(game, new_x, new_y)) {
        game->player_x = new_x;
        game->player_y = new_y;
        game->energy_consumed += game->step_cost;

        // 检查是否踩到陷阱
        if (game->map[new_y][new_x] == TRAP) {
            game->step_cost = 2;
        } else {
            game->step_cost = 1;
        }

        // 检查宝藏
        check_treasure(game);

    } else if (toupper(direction) != 'I' && toupper(direction) != 'Q') {
        // 撞墙，消耗体力但不移动
        game->energy_consumed += game->step_cost;
    }
}

// 检查宝藏
void check_treasure(GameState *game) {
    for (int i = 0; i < game->treasure_count; i++) {
        if (game->treasures[i][0] == -1) continue; // 已找到的宝藏

        int tx = game->treasures[i][0];
        int ty = game->treasures[i][1];

        // 检查上下左右四个方向
        if ((abs(game->player_x - tx) == 1 && game->player_y == ty) ||
            (abs(game->player_y - ty) == 1 && game->player_x == tx)) {
            game->found_treasures++;
            game->treasures[i][0] = -1; // 标记为已找到

            printf("找到了一个宝藏！还剩下 %d 个宝藏。\n", game->treasure_count - game->found_treasures);

            if (game->found_treasures >= game->treasure_count) {
                game->game_over = 1;
                game_summary(game, "找到所有宝藏");
            }
            break;
        }
    }
}

// 游戏总结
void game_summary(GameState *game, const char *reason) {
    CLEAR_SCREEN();
    printf("========================================\n");
    printf("          小黄的寻宝之旅结束          \n");
    printf("========================================\n");
    printf("结束原因：%s\n", reason);
    printf("行动路径：%s\n", game->path);
    printf("消耗的体力：%d\n", game->energy_consumed);
    printf("找到的宝箱的数量：%d\n", game->found_treasures);
    printf("\n<按任意键继续>\n");
    getch();
}

// 处理编程模式
void process_programming_mode(GameState *game) {
    char input[100];
    printf("请输入移动序列：");
    if (fgets(input, sizeof(input), stdin)) {
        for (int i = 0; input[i] != '\0' && input[i] != '\n'; i++) {
            if (game->game_over) break;
            move_player(game, input[i]);
            render_game(game);
            // 添加延迟以便观察
#ifdef _WIN32
            Sleep(500);
#else
            usleep(500000);
#endif
        }
    }
}

// 主函数
int main() {
    GameState game;
    int running = 1;

    while (running) {
        // 欢迎界面
        int menu_selection = 0;
        int level_choice = 0;

        while (1) {
            render_welcome_screen();

            char input = getch();
            switch (toupper(input)) {
                case 'W':
                    if (menu_selection > 0) menu_selection--;
                    break;
                case 'S':
                    if (menu_selection < 2) menu_selection++;
                    break;
                case '\r': // Enter键
                    level_choice = menu_selection + 1;
                    break;
            }

            // 更新菜单显示
            CLEAR_SCREEN();
            printf("========================================\n");
            printf("          小黄的奇妙探险！            \n");
            printf("========================================\n");
            printf("\n");
            for (int i = 0; i < 3; i++) {
                if (i == menu_selection) {
                    printf("> ");
                } else {
                    printf("  ");
                }

                switch (i) {
                    case 0: printf("开始第一关\n"); break;
                    case 1: printf("开始第二关\n"); break;
                    case 2: printf("退出\n"); break;
                }
            }
            printf("\n控制方法：按 W 向上移动，按 S 向下移动，按 <Enter> 选择\n");

            if (level_choice != 0) break;
        }

        if (level_choice == 3) {
            break; // 退出游戏
        }

        // 模式选择
        int mode_selection = 0;
        ControlMode selected_mode = REALTIME_MODE;

        while (1) {
            render_mode_selection();

            char input = getch();
            switch (toupper(input)) {
                case 'W':
                    if (mode_selection > 0) mode_selection--;
                    break;
                case 'S':
                    if (mode_selection < 1) mode_selection++;
                    break;
                case '\r': // Enter键
                    selected_mode = (ControlMode)mode_selection;
                    break;
            }

            // 更新模式选择显示
            CLEAR_SCREEN();
            printf("请选择控制模式：\n");
            for (int i = 0; i < 2; i++) {
                if (i == mode_selection) {
                    printf("> ");
                } else {
                    printf("  ");
                }
                printf("%d: %s模式\n", i, i == 0 ? "实时" : "编程");
            }
            printf("\n控制方法：按 W 向上移动，按 S 向下移动，按 <Enter> 选择\n");

            if (selected_mode == mode_selection) break;
        }

        // 加载关卡并初始化游戏
        load_level(&game, level_choice);
        game.control_mode = selected_mode;
        initialize_game(&game);

        // 游戏主循环
        while (!game.game_over) {
            render_game(&game);

            if (game.control_mode == REALTIME_MODE) {
                char input = getch();
                move_player(&game, input);
            } else {
                process_programming_mode(&game);
            }
        }
    }

    printf("感谢游玩！再见！\n");
    return 0;
}