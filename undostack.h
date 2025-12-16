#ifndef UNDO_STACK_H
#define UNDO_STACK_H

#include "gamestate.h"

#define MAX_UNDO_STACK 100

typedef struct UndoNode {
    GameState state;
    struct UndoNode* prev;
    struct UndoNode* next;
} UndoNode;

typedef struct {
    UndoNode* current;
    UndoNode* head;
    UndoNode* tail;
    int count;//计数并防止溢出
} UndoStack;//undoStack 负责 “管理所有 UndoNode 组成的链

void undo_stack_init(UndoStack* stack);
void undo_stack_push(UndoStack* stack, const GameState* state);
int undo_stack_undo(UndoStack* stack, GameState* state);
int undo_stack_redo(UndoStack* stack, GameState* state);
void undo_stack_clear_after(UndoStack* stack);
void undo_stack_free(UndoStack* stack);

#endif
