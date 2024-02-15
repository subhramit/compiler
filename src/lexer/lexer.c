/*
#                    GROUP - 8
# 2020B1A70630P                       Aditya Thakur
# 2021A7PS2001P                       Amal Sayeed
# 2020A7PS2005P                       Ohiduz Zaman
# 2020A7PS2682P                       Priyansh Patel
# 2020A7PS2002P                       Rachoita Das
# 2020B1A70611P                       Subhramit Basu Bhowmick
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
//#include "lexerDef.h"

//FILE* getStream(FILE* fp) {
    /*
    This function takes the input from the file pointed to by 'fp'.
    This file is the source code written in the given language.
    The function uses efficient technique to populate twin buffer by bringing the fixed sized piece of source code into the memory for processing so as to avoid intensive I/O operations mixed with CPU intensive tasks.
    The function also maintains the file pointer after every access so that it can get more data into the memory on demand.
    The implementation can also be combined with getNextToken() implementation as per the convenience of the team.
    */
   
//}

//tokenInfo getNextToken(twinBuffer B) {
    /*
    This function reads the input character stream and uses efficient mechanism to recognize lexemes.
    The function tokenizes the lexeme appropriately and returns all relevant information it collects in this phase (lexical analysis phase) encapsulated as tokenInfo. 
    The function also displays lexical errors appropriately.
    */

//}

void removeComments(char* testcaseFile, char* cleanFile) {
    /*
    This function is an additional plug in to clean the source code by removal of comments.
    The function takes as input the source code and writes the clean code in the file appropriately.
    [Note: This function is invoked only once through your driver file to showcase the comment removal for evaluation purpose.
    However, your lexer does not really pick inputs from comment removed file.
    Rather, it keeps ignoring the comments and keep collecting token Info to pass to the parser.
    For showcasing your lexers ability, directly take input from user source code]
    */

    FILE* inputfile = fopen(testcaseFile, "r"); //should be changed to "rb" if our language has a different file extension

    if(inputfile == NULL) {
        printf("Error opening the specified file!\n");
        return;
    }

    printf("No errors 1 and 2\n");
    char buf[2048]; //buffer to temporarily hold lines
    FILE* outputfile = fopen(cleanFile, "w");
    char* commentSym;
    
    while(fgets(buf, sizeof(buf), inputfile) != NULL) { //loop through each line

        commentSym = strchr(buf, '%'); //strchr finds the position of % in the line
        if (commentSym != NULL) { //not a null pointer means % found
            *commentSym = '\n'; //change line
            *(commentSym+1) = '\0'; //truncate the rest of the line
        }

        fputs(buf, outputfile); //copy the modified line
    }

    fclose(inputfile);
    fclose(outputfile);

}

void printCleanFile(char* cleanFile) {

    FILE* printable = fopen(cleanFile, "r");

    if(printable == NULL) {
        printf("Error opening cleaned file!\n");
        exit(-1);
    }

    char ch;
    ch = fgetc(printable);
    while (ch != EOF) {
        printf ("%c", ch); 
        ch = fgetc(printable); 
    }

    fclose(printable);

}