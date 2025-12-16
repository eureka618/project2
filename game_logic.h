#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "gamestate.h"
#include "undostack.h"

void game_init(GameState* state, int level_index);
void game_cleanup(GameState* state);
int move_player(GameState* state, char direction, UndoStack* undo_stack);
int is_valid_move(const GameState* state, int x, int y);
void handle_tile_interaction(GameState* state, int x, int y);
void check_treasure_collection(GameState* state);
int check_game_completion(GameState* state);
void reset_game(GameState* state);
void process_realtime_input(GameState* state, UndoStack* undo_stack);
void process_programming_input(GameState* state, UndoStack* undo_stack);

#endif