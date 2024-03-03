#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TokenSet.h"
#define MAX_TOKEN_LENGTH 50
#define INITIAL_CAPACITY 30
#define LOAD_FACTOR 0.75

typedef struct Node {
    char key[MAX_TOKEN_LENGTH];
    struct Node* next;
} Node;

typedef struct {
    Node** slots;
    int capacity;
    int size;
} TokenSet;

unsigned int hash_func(const char* key, int capacity) {
    unsigned int total = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        total += (unsigned int)key[i];
    }
    return total % capacity;
}

TokenSet* createTokenSet() {
    TokenSet* ts = (TokenSet*)malloc(sizeof(TokenSet));
    if (ts) {
        ts->capacity = INITIAL_CAPACITY;
        ts->size = 0;
        ts->slots = (Node**)malloc(sizeof(Node*) * ts->capacity);
        if (!ts->slots) {
            free(ts);
            return NULL;
        }
        for (int i = 0; i < ts->capacity; i++) {
            ts->slots[i] = NULL;
        }
    }
    return ts;
}

void rehash(TokenSet* ts) {
    int old_capacity = ts->capacity;
    ts->capacity *= 2;
    Node** old_slots = ts->slots;
    ts->slots = (Node**)malloc(sizeof(Node*) * ts->capacity);
    if (!ts->slots) {
        // Memory allocation failed, cleanup and return
        free(old_slots);
        free(ts);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < ts->capacity; i++) {
        ts->slots[i] = NULL;
    }
    // Reinsert elements into new slots
    for (int i = 0; i < old_capacity; i++) {
        Node* current = old_slots[i];
        while (current != NULL) {
            Node* next = current->next;
            unsigned int index = hash_func(current->key, ts->capacity);
            current->next = ts->slots[index];
            ts->slots[index] = current;
            current = next;
        }
    }
    free(old_slots);
}

void addToken(TokenSet* ts, const char* tok) {
    unsigned int index = hash_func(tok, ts->capacity);
    Node* current = ts->slots[index];
    while (current != NULL) {
        if (strcmp(current->key, tok) == 0) {
            return; // Token already exists, do nothing
        }
        current = current->next;
    }
    // Token does not exist, insert it
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        // Memory allocation failed, cleanup and return
        free(ts);
        exit(EXIT_FAILURE);
    }
    strcpy(newNode->key, tok);
    newNode->next = ts->slots[index];
    ts->slots[index] = newNode;
    ts->size++;
    // Check if rehashing is needed
    if ((float)ts->size / ts->capacity > LOAD_FACTOR) {
        rehash(ts);
    }
}

void removeToken(TokenSet* ts, const char* tok) {
    unsigned int index = hash_func(tok, ts->capacity);
    Node* current = ts->slots[index];
    Node* prev = NULL;
    while (current != NULL) {
        if (strcmp(current->key, tok) == 0) {
            if (prev != NULL) {
                prev->next = current->next;
            } else {
                ts->slots[index] = current->next;
            }
            free(current);
            ts->size--;
            return;
        }
        prev = current;
        current = current->next;
    }
}

int isMember(TokenSet* ts, const char* tok) {
    unsigned int index = hash_func(tok, ts->capacity);
    Node* current = ts->slots[index];
    while (current != NULL) {
        if (strcmp(current->key, tok) == 0) {
            return 1; // Token found
        }
        current = current->next;
    }
    return 0; // Token not found
}

void destroyTokenSet(TokenSet* ts) {
    for (int i = 0; i < ts->capacity; i++) {
        Node* current = ts->slots[i];
        while (current != NULL) {
            Node* next = current->next;
            free(current);
            current = next;
        }
    }
    free(ts->slots);
    free(ts);
}