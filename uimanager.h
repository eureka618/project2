#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "gamestate.h"
#include "file_io.h"

void clear_screen();
void print_header(const char* text);
void print_menu_item(int index, const char* text, int selected);
void print_info(const char* text);
void print_hint(const char* text);

void render_main_menu(GameState* state);
void render_level_menu(GameState* state);
void render_mode_menu(GameState* state);
void render_load_save_menu(GameState* state, const SaveInfo* info);
void render_game(GameState* state);
void render_summary(GameState* state);

char get_user_input();
char get_char_input();
void get_string_input(char* buffer, int max_len);

#endif