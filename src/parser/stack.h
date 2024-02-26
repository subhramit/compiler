#ifndef STACK_H
#define STACK_H

typedef struct stackNode {
    grammarSymbol* data;
    struct stackNode* next;
} stackNode;

typedef stackNode* stackNODE;

typedef struct stack {
    stackNODE top;
} Stack;

Stack* createNewStack();
void push(Stack* stack, grammarSymbol* gs);
void pop(Stack* stack);
grammarSymbol* top(Stack* stack);
bool isEmpty(Stack* stack);

#endif