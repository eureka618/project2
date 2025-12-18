#include <stdio.h>
#include <stdlib.h>
#include "gamestate.h"
#include "game_logic.h"
#include "uimanager.h"
#include "file_io.h"
#include "undostack.h"
#include "utils.h"
void handle_level_menu(GameState* state) ;
void handle_main_menu(GameState* state) ;
void handle_load_save_menu(GameState* state, const SaveInfo* info) ;
void handle_mode_menu(GameState* state) ;
int main() {
    GameState state = {0};
    UndoStack undo_stack;

    // 初始化
    undo_stack_init(&undo_stack);
    create_directory(MAPS_DIR);
    create_directory(SAVES_DIR);

    // 加载地图列表
    if (!load_map_list(&state)) {
        printf("错误：无法加载地图列表\n");
        return 1;
    }

    state.current_screen = MENU_MAIN;

    // 主循环
    while (1) {
        switch (state.current_screen) {
            case MENU_MAIN:
                render_main_menu(&state);
                handle_main_menu(&state);
                break;

            case MENU_LEVEL:
                render_level_menu(&state);
                handle_level_menu(&state);
                break;

            case MENU_LOAD_SAVE: {
                SaveInfo info = get_save_info(state.level_names[state.menu_selection]);
                render_load_save_menu(&state, &info);
                handle_load_save_menu(&state, &info);
                break;
            }

            case MENU_MODE:
                render_mode_menu(&state);
                handle_mode_menu(&state);
                break;

            case GAME_PLAYING:
                render_game(&state);
                if (state.session.control_mode == REALTIME_MODE) {
                    process_realtime_input(&state, &undo_stack);
                } else {
                    process_programming_input(&state, &undo_stack);
                }

                if (state.session.game_over) {
                    save_game(&state);
                    state.current_screen = GAME_SUMMARY;
                }
                break;

            case GAME_SUMMARY:
                render_summary(&state);
                printf("\n按任意键返回主菜单...");
                get_char_input();
                state.current_screen = MENU_MAIN;
                undo_stack_free(&undo_stack);
                undo_stack_init(&undo_stack);
                break;
        }
    }

    undo_stack_free(&undo_stack);
    return 0;
}

void handle_main_menu(GameState* state) {
    char input = get_user_input();

    switch (toupper(input)) {
        case 'W':
            if (state->menu_selection > 0) state->menu_selection--;
            break;
        case 'S':
            if (state->menu_selection < state->level_count) state->menu_selection++;
            break;
        case '\r': // Enter
            if (state->menu_selection == state->level_count) {
                // 退出
                exit(0);
            } else {
                state->current_screen = MENU_LOAD_SAVE;
            }
            break;
    }
}

void handle_load_save_menu(GameState* state, const SaveInfo* info) {
    char input = get_user_input();
    static int load_save_selection=0;

    switch (toupper(input)) {
        case 'W':
            if (load_save_selection > 0) load_save_selection--;
            break;
        case 'S':
            if (load_save_selection< 1) load_save_selection++;
            break;
        case '\r': // Enter
            if (load_save_selection == 0) {
                // 加载存档--使用当前选中的关卡
                if (load_save(state, state->level_names[state->menu_selection])) {
                    state->current_screen = GAME_PLAYING;
                }
            } else {
                // 重新开始
                clear_save(state->level_names[state->menu_selection]);
                state->current_screen = MENU_MODE;
                state->session.current_level = state->menu_selection;
            }
            load_save_selection=0;
            break;
    }
}

void handle_mode_menu(GameState* state) {
    char input = get_user_input();
    static int mode_selection=0;

    switch (toupper(input)) {
        case 'W':
            if (mode_selection > 0)  mode_selection--;
            break;
        case 'S':
            if (mode_selection < 1) mode_selection++;
            break;
        case '\r': // Enter
            state->session.control_mode = (ControlMode)state->menu_selection;
            game_init(state, state->menu_selection);
            state->current_screen = GAME_PLAYING;
            mode_selection=0;
            break;
    }
}
void handle_level_menu(GameState* state) {
    char input = get_user_input();  // 获取用户输入（W/S/Enter）

    switch (toupper(input)) {
        case 'W':
            // 上移选择项（防止越界）
            if (state->menu_selection > 0) {
                state->menu_selection--;
            }
            break;
        case 'S':
            // 下移选择项（防止越界）
            if (state->menu_selection < state->level_count - 1) {
                state->menu_selection++;
            }
            break;
        case '\r':  // 回车键确认选择
            // 选中关卡后，跳转到加载存档菜单
            state->current_screen = MENU_LOAD_SAVE;
            break;
    }
}