#ifndef STACK_H
#define STACK_H

typedef struct node {
    grammarSymbol* data;
    struct node* next;
} Node;

typedef Node* NODE;

typedef struct stack {
    NODE top;
} Stack;

Stack* createNewStack();
void push(Stack* stack, grammarSymbol* gs);
void pop(Stack* stack);
grammarSymbol* top(Stack* stack);
int isEmpty(Stack* stack);

#endif