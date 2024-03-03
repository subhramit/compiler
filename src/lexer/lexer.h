/*
#                    GROUP - 8
# 2020B1A70630P                       Aditya Thakur
# 2021A7PS2001P                       Amal Sayeed
# 2020A7PS2005P                       Ohiduz Zaman
# 2020A7PS2682P                       Priyansh Patel
# 2020A7PS2002P                       Rachoita Das
# 2020B1A70611P                       Subhramit Basu Bhowmick
*/

// Contains function prototype declarations of functions in lexer.c

#ifndef LEXER_PROTOTYPES
#define LEXER_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "lexerDef.h"


TrieNode* createTrieNode();
Trie* createTrie();
void insertKeyword(Trie* myTrie, const char* word, Token tkType);
Token searchKeyword(Trie* myTrie, const char* word);
SymbolTable* createSymbolTable();
void insertToken(SymbolTable* ST, SymbolTableEntry* stEntry);
SymbolTableEntry* createToken(char* lxm, Token tkType, double valNum);
SymbolTableEntry* searchToken(SymbolTable* ST, char* lxm);
linkedList* createNewList();
tokenInfo* createNewNode(SymbolTableEntry* ste, int lineNo);
void insertLLNode(linkedList* myList, tokenInfo* myNode);
linkedList* LexInput(FILE* fp);
//tokenInfo getNextToken(twinBuffer B);
void removeComments(char* testcaseFile, char* cleanFile);
void printCleanFile(char* cleanFile);
linkedList* getAllTokens(FILE* fp);
void initializeKeywordsLookup(Trie* keywordsLookup);
tokenInfo* getNextToken(FILE* fp, char *twinBuff, int *fwdPtr, int *lineNumber, Trie* keywordsLookup, SymbolTable* symbolTable);
void tokenizeLexeme(int beginPtr, int* fwdPtr, char* lexeme, char* twinBuff);
char nextChar(FILE* fp, char *twinBuff, int *fwdPtr);
void initializeTokenToString();

#endif