#include "undostack.h"
#include <stdlib.h>
#include <string.h>

void undo_stack_init(UndoStack* stack) {
    stack->current = NULL;
    stack->head = NULL;
    stack->tail = NULL;
    stack->count = 0;
}

void undo_stack_push(UndoStack* stack, const GameState* state) {
    // 清除当前节点之后的所有节点（当有新操作时）
    undo_stack_clear_after(stack);

    UndoNode* new_node = (UndoNode*)malloc(sizeof(UndoNode));
    if (!new_node) return;

    // 复制状态
    memcpy(&new_node->state, state, sizeof(GameState));
    new_node->prev = stack->tail;
    new_node->next = NULL;

    if (stack->tail) {
        stack->tail->next = new_node;
    } else {
        stack->head = new_node;
    }

    stack->tail = new_node;
    stack->current = new_node;
    stack->count++;

    // 限制栈大小
    if (stack->count > MAX_UNDO_STACK) {
        UndoNode* to_remove = stack->head;
        stack->head = to_remove->next;
        if (stack->head) stack->head->prev = NULL;
        free(to_remove);
        stack->count--;
    }
}

int undo_stack_undo(UndoStack* stack, GameState* state) {
    // 检查是否可以撤销（当前节点存在且不是第一个节点）
    if (!stack->current || !stack->current->prev) {
        return 0;
    }

    // 移动到前一个状态
    stack->current = stack->current->prev;
    // 恢复状态
    memcpy(state, &stack->current->state, sizeof(GameState));
    return 1;
}

int undo_stack_redo(UndoStack* stack, GameState* state) {
    // 检查是否可以恢复（当前节点存在且不是最后一个节点）
    if (!stack->current || !stack->current->next) {
        return 0;
    }

    // 移动到后一个状态
    stack->current = stack->current->next;
    // 恢复状态
    memcpy(state, &stack->current->state, sizeof(GameState));
    return 1;
}

void undo_stack_clear_after(UndoStack* stack) {
    if (!stack->current) return;

    // 删除当前节点之后的所有节点
    UndoNode* node = stack->current->next;
    while (node) {
        UndoNode* next = node->next;
        free(node);
        node = next;
        stack->count--;
    }

    // 断开链接
    stack->current->next = NULL;
    stack->tail = stack->current;
}

void undo_stack_free(UndoStack* stack) {
    UndoNode* node = stack->head;
    while (node) {
        UndoNode* next = node->next;
        free(node);
        node = next;
    }

    stack->head = NULL;
    stack->tail = NULL;
    stack->current = NULL;
    stack->count = 0;
}