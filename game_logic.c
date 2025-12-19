#include "game_logic.h"
#include "uimanager.h"
#include <stdio.h>
#include <string.h>
#include<math.h>
#include <ctype.h>


void game_init(GameState* state, int level_index) {
    // 重置游戏会话
    memset(&state->session, 0, sizeof(GameSession));//初始化信息

    // 设置最后游玩关卡
    strcpy(state->last_played_level, state->level_names[level_index]);
    // 加载地图
    if (!load_map(state, state->level_names[level_index])) {
        // 错误处理
        return;
    }

    // 初始化玩家
    state->session.player.x = state->map.start_x;
    state->session.player.y = state->map.start_y;
    state->session.player.energy = 0;
    state->session.player.step_cost = 1;

    // 初始化游戏状态
    state->session.total_treasures = state->map.treasure_count;
    state->session.treasures_found = 0;
    state->session.energy_consumed = 0;
    state->session.path_length = 0;
    state->session.current_level = level_index;
    state->session.game_over = 0;
    state->session.start_time = time(NULL);

    memset(state->session.movement_path, 0, MAX_PATH);
}

int move_player(GameState* state, char direction, UndoStack* undo_stack) {
    int new_x = state->session.player.x;
    int new_y = state->session.player.y;
    if (toupper(direction) != 'Z' && toupper(direction) != 'Y') {
        // 只有普通移动才保存当前状态到撤销栈
        undo_stack_push(undo_stack, state);
    }

    switch (toupper(direction)) {
        case 'W': new_y--; break;
        case 'S': new_y++; break;
        case 'A': new_x--; break;
        case 'D': new_x++; break;
        case 'I': break; // 原地不动
        case 'Z': // 撤销
            if (undo_stack_undo(undo_stack, state)) {
                return 1;
            }
            return 0;
        case 'Y': // 恢复
            if (undo_stack_redo(undo_stack, state)) {
                return 1;
            }
            return 0;
        case 'Q':
            state->session.game_over = 1;
            return 1;
        default:
            if (state->session.control_mode == REALTIME_MODE) {
                return 0; // 忽略非法输入
            } else {
                state->session.game_over = 1;
                return -1; // 编程模式下终止
            }
    }

    // 除了撤销/恢复操作外，其他移动操作需要保存状态
    if (direction != 'I'&&direction != 'Z' && direction != 'Y') {
        // 保存当前状态到撤销栈
        undo_stack_push(undo_stack, state);
    }

    // 记录移动路径（除原地不动外）
    if (direction != 'I' && direction != 'Z' && direction != 'Y') {
        state->session.movement_path[state->session.path_length] = toupper(direction);
        state->session.path_length++;
    }

    // 检查移动是否有效
    if (is_valid_move(state, new_x, new_y)) {
        // 消耗体力
        state->session.energy_consumed += state->session.player.step_cost;

        // 更新位置
        state->session.player.x = new_x;
        state->session.player.y = new_y;

        // 处理地块交互
        handle_tile_interaction(state, new_x, new_y);

        // 检查宝藏收集
        check_treasure_collection(state);

        // 检查游戏完成条件
        if (check_game_completion(state)) {
            state->session.game_over = 1;
        }

    } else if (toupper(direction) != 'I') {
        // 撞墙，消耗体力但位置不变
        state->session.energy_consumed += state->session.player.step_cost;
    }

    return 1;
}


int is_valid_move(const GameState* state, int x, int y) {
    if (x < 0 || x >= state->map.width ||
        y < 0 || y >= state->map.height) {
        return 0;
    }

    return state->map.map[y][x] != WALL;
}

void handle_tile_interaction(GameState* state, int x, int y) {
    switch (state->map.map[y][x]) {
        case TRAP:
            state->session.player.step_cost = 2;
            break;
        default:
            state->session.player.step_cost = 1;
            break;
    }
}

void check_treasure_collection(GameState* state) {
    for (int i = 0; i < state->map.treasure_count; i++) {
        if (state->map.treasures[i].collected) continue;

        int tx = state->map.treasures[i].x;
        int ty = state->map.treasures[i].y;

        // 检查玩家是否与宝藏位置重合
        if (state->session.player.x == tx && state->session.player.y == ty) {

            state->map.treasures[i].collected = 1;
            state->session.treasures_found++;

            // 从地图上移除宝藏显示
            state->map.map[ty][tx] = EMPTY;
        }
    }
}

int check_game_completion(GameState* state) {
    return state->session.treasures_found >= state->session.total_treasures;
}

void process_realtime_input(GameState* state, UndoStack* undo_stack) {
    char input = get_char_input();
    move_player(state, input, undo_stack);
}

void process_programming_input(GameState* state, UndoStack* undo_stack) {
    char sequence[100];
    printf("请输入指令序列: ");
    fgets(sequence, sizeof(sequence), stdin);

    for (int i = 0; sequence[i] != '\0' && sequence[i] != '\n'; i++) {
        if (state->session.game_over) break;

        int result = move_player(state, sequence[i], undo_stack);
        if (result == -1) {
            // 编程模式下非法输入，终止游戏
            state->session.game_over = 1;
            break;
        }

        // 显示每一步的状态
        render_game(state);
    }
}