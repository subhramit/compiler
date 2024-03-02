#ifndef STACK_H
#define STACK_H

typedef struct stackNode {
    pTreeNode* data;
    struct stackNode* next;
} stackNode;

typedef stackNode* stackNODE;

typedef struct stack {
    stackNODE top;
} Stack;

Stack* createNewStack();
void push(Stack* stack, pTreeNode* ptn);
void pop(Stack* stack);
pTreeNode* top(Stack* stack);
bool isEmpty(Stack* stack);

#endif