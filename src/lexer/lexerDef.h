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
#include <string.h>
#include <stdbool.h>

#define ALPHABET_SIZE 26
#define INITIAL_SYMBOL_TABLE_CAPACITY 10
#define BUFFER_SIZE 256
#define TOKEN_NAME_LENGTH 50

typedef enum Token {
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
    NE,
    EPS,
    DOLLAR, // End of File marker
    LEXICAL_ERROR,
    TK_NOT_FOUND
} Token;


char* tokenToString[TK_NOT_FOUND];

// Trie Node
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    int isEndOfWord; // Flag to indicate end of word
    Token tokenType;
} TrieNode;

//Trie
typedef struct Trie{
    struct TrieNode* root;
} Trie;

// Each entry in the symbol table
typedef struct SymbolTableEntry{
    char lexeme[BUFFER_SIZE];
    Token tokenType;
    double valueIfNumber;
} SymbolTableEntry;

// The Symbol Table 
typedef struct SymbolTable{
    int capacity, size;
    SymbolTableEntry** table;
} SymbolTable;

typedef struct llnode{
    SymbolTableEntry* STE;
    int lineNumber;
    struct llnode* next;
} tokenInfo;

typedef struct linkedList{
    int count;
    tokenInfo *head, *tail;
} linkedList;

#endif
