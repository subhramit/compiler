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

#define ALPHABET_SIZE 26
#define INITIAL_SYMBOL_TABLE_CAPACITY 10
#define BUFFER_SIZE 40

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
    DOLLAR, // End of File marker
    LEXICAL_ERROR,
    NOT_FOUND
} Token;






// Trie Node
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    int isEndOfWord; // Flag to indicate end of word
    Token tokenType;
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
        printf("Could not allocate memory for creating trie node\n");
    }
    return newNode;
}

//Trie
typedef struct Trie{
    struct TrieNode* root;
} Trie;

// Function to create a new Trie
Trie* createTrie(){
    Trie* newTrie = (Trie*) malloc(sizeof(Trie));
    if(newTrie){
        newTrie->root = createTrieNode();
    }
    else{
        printf("Could not allocate memory for creating trie\n");
    }
    return newTrie;
}

// Function to insert a keyword into the Trie
void insertKeyword(Trie* myTrie, const char* word, Token tkType) {
    TrieNode* currentNode = myTrie->root;
    for (int i = 0; word[i] != '\0'; i++) {
        int index = word[i] - 'a';
        if (!(currentNode->children[index])) {
            currentNode->children[index] = createTrieNode();
        }
        currentNode = currentNode->children[index];
    }
    currentNode->isEndOfWord = 1;
    currentNode->tokenType = tkType;
}

// Function to search for a keyword in the Trie and return the corresponding token
Token searchKeyword(Trie* myTrie, const char* word) {
    TrieNode* currentNode = myTrie->root;
    for (int i = 0; word[i] != '\0'; i++) {
        int index = word[i] - 'a';
        if (!(currentNode->children[index])) {
            return NOT_FOUND; // Word not found
        }
        currentNode = currentNode->children[index];
    }
    if(currentNode && currentNode->isEndOfWord)
        return currentNode->tokenType;
    return NOT_FOUND;
}






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


// Create a new Symbol Table
SymbolTable* createSymbolTable(){
    SymbolTable* newSymbolTable = (SymbolTable*) malloc(sizeof(SymbolTable));
    if(!(newSymbolTable)){
        printf("Could not allocate memory for Symbol Table struct\n"); return NULL;
    }
    newSymbolTable->table = (SymbolTableEntry**) malloc(INITIAL_SYMBOL_TABLE_CAPACITY*sizeof(SymbolTableEntry*));
    if(!(newSymbolTable->table)){
        printf("Could not allocate memory for creating Symbol Table\n"); 
    }
    newSymbolTable->capacity = INITIAL_SYMBOL_TABLE_CAPACITY;
    newSymbolTable->size=0;
    return newSymbolTable;
}


// Insert a token and other details as an entry into the Symbol Table
void insertToken(SymbolTable* ST, SymbolTableEntry* stEntry){
    if(ST->size == ST->capacity){
        ST->capacity *= 2;
        ST->table = (SymbolTableEntry**) realloc(ST->table , ST->capacity*sizeof(SymbolTableEntry*));
        if(!(ST->table)){
            printf("Could not allocate memory for resizing Symbol Table\n");
            return;
        }
    }
    ST->table[(ST->size)++] = stEntry;
}


// Create a token / an entry in the Symbol Table
SymbolTableEntry* createToken(char* lxm, Token tkType, double valNum){
    SymbolTableEntry* newTok = (SymbolTableEntry*) malloc(sizeof(SymbolTableEntry));
    if(!(newTok)){
        printf("Could not allocate memory for creating a new token\n");
        return NULL;
    }
    strcpy(newTok->lexeme, lxm); 
    newTok->tokenType = tkType;
    newTok->valueIfNumber = valNum;
    return newTok;
}

// Search for a lexeme in the Symbol Table
SymbolTableEntry* searchToken(SymbolTable* ST, char* lxm){
    for(int i=0; i<ST->size; ++i)
        if(!strcmp(lxm, ST->table[i]->lexeme)) 
            return ST->table[i];
    return NULL;
}





typedef struct llnode{
    SymbolTableEntry* STE;
    int lineNumber;
    struct llnode* next;
} tokenInfo;


typedef struct linkedList{
    int count;
    tokenInfo *head, *tail;
} linkedList;


linkedList* createNewList(){
    linkedList* myList = (linkedList*) malloc(sizeof(linkedList));
    if(!(myList)){
        printf("Could not allocate memory for creation of linked list\n");
        return NULL;
    }
    myList->count=0;
    myList->head=NULL;
    myList->tail=NULL;
    return myList;
}


tokenInfo* createNewNode(SymbolTableEntry* ste, int lineNo){
    tokenInfo* myNode = (tokenInfo*) malloc(sizeof(tokenInfo));
    if(!(myNode)){
        printf("Could not allocate memory for linked list node\n");
        return NULL;
    }
    myNode->STE = ste;
    myNode->lineNumber = lineNo;
    myNode->next = NULL;
    return myNode;
}


void insertLLNode(linkedList* myList, tokenInfo* myNode){
    if(!(myList->count)){
        myList->head = myNode;
        myList->tail = myNode;
    }
    else{
        myList->tail->next = myNode;
        myList->tail = myNode;
    }
    ++(myList->count);
}


#endif