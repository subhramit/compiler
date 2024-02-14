#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* getStream(FILE* fp) {
    /*
    This function takes the input from the file pointed to by 'fp'.
    This file is the source code written in the given language.
    The function uses efficient technique to populate twin buffer by bringing the fixed sized piece of source code into the memory for processing so as to avoid intensive I/O operations mixed with CPU intensive tasks.
    The function also maintains the file pointer after every access so that it can get more data into the memory on demand.
    The implementation can also be combined with getNextToken() implementation as per the convenience of the team.
    */
   
}

tokenInfo getNextToken(twinBuffer B) {
    /*
    This function reads the input character stream and uses efficient mechanism to recognize lexemes.
    The function tokenizes the lexeme appropriately and returns all relevant information it collects in this phase (lexical analysis phase) encapsulated as tokenInfo. 
    The function also displays lexical errors appropriately.
    */

}

removeComments(char* testcaseFile, char* cleanFile) {
    /* This function is an additional plug in to clean the source code by removal of comments.
    The function takes as input the source code and writes the clean code in the file appropriately.
    [Note: This function is invoked only once through your driver file to showcase the comment removal for evaluation purpose.
    However, your lexer does not really pick inputs from comment removed file.
    Rather, it keeps ignoring the comments and keep collecting token Info to pass to the parser.
    For showcasing your lexers ability, directly take input from user source code]
    */

}