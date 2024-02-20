/*
#                    GROUP - 8
# 2020B1A70630P                       Aditya Thakur
# 2021A7PS2001P                       Amal Sayeed
# 2020A7PS2005P                       Ohiduz Zaman
# 2020A7PS2682P                       Priyansh Patel
# 2020A7PS2002P                       Rachoita Das
# 2020B1A70611P                       Subhramit Basu Bhowmick
*/

// Contains all data definitions used in lexer.c

#ifndef LEXER_DEF
#define LEXER_DEF

#include <stdio.h>
#include <stdlib.h>

#define ALPHABET_SIZE 26

enum Token {
    ASSIGNOP,
    COMMENT,
    FIELDID,
    ID,
    NUM,
    RNUM,
    FUNID,
    RUID,
    WITH,
    PARAMETERS,
    END,
    WHILE,
    UNION,
    ENDUNION,
    DEFINETYPE,
    AS,
    TYPE,
    MAIN,
    GLOBAL,
    PARAMETER,
    LIST,
    SQL,
    SQR,
    INPUT,
    OUTPUT,
    INT,
    REAL,
    COMMA,
    SEM,
    COLON,
    DOT,
    ENDWHILE,
    OP,
    CL,
    IF,
    THEN,
    ENDIF,
    READ,
    WRITE,
    RETURN,
    PLUS,
    MINUS,
    MUL,
    DIV,
    CALL,
    RECORD,
    ENDRECORD,
    ELSE,
    AND,
    OR,
    NOT,
    LT,
    LE,
    EQ,
    GT,
    GE,
    NE
};

// Trie Node
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    int isEndOfWord; // Flag to indicate end of word
    enum Token tokenType;
} TrieNode;

// Function to create a new Trie node
TrieNode* createTrieNode() {
    TrieNode* newNode = (TrieNode*)malloc(sizeof(TrieNode));
    if (newNode) {
        newNode->isEndOfWord = 0;
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            newNode->children[i] = NULL;
        }
    }
    else{
        printf("Could not allocate memory for creating node\n");
    }
    return newNode;
}

// Function to insert a word into the Trie
void insertKeyword(TrieNode* root, const char* word, enum Token tkType) {
    TrieNode* currentNode = root;
    for (int i = 0; word[i] != '\0'; i++) {
        int index = word[i] - 'a';
        if (!currentNode->children[index]) {
            currentNode->children[index] = createTrieNode();
        }
        currentNode = currentNode->children[index];
    }
    currentNode->isEndOfWord = 1;
    currentNode->tokenType = tkType;
}

// Function to search for a word in the Trie
int searchKeyword(TrieNode* root, const char* word) {
    TrieNode* currentNode = root;
    for (int i = 0; word[i] != '\0'; i++) {
        int index = word[i] - 'a';
        if (!currentNode->children[index]) {
            return -1; // Word not found
        }
        currentNode = currentNode->children[index];
    }
    if(currentNode && currentNode->isEndOfWord)
        return currentNode->tokenType;
    return -1;
}

#endif