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
#include "lexerDef.h"

//FILE* getStream(FILE* fp);
//tokenInfo getNextToken(twinBuffer B);
void removeComments(char* testcaseFile, char* cleanFile);
void printCleanFile(char* cleanFile);

#endif