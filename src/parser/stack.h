/*
#                    GROUP - 8
# 2020B1A70630P                       Aditya Thakur
# 2021A7PS2001P                       Amal Sayeed
# 2020A7PS2005P                       Ohiduz Zaman
# 2020A7PS2682P                       Priyansh Patel
# 2020A7PS2002P                       Rachoita Das
# 2020B1A70611P                       Subhramit Basu Bhowmick
*/

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