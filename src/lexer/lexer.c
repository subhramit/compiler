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
#include "lexer.h"
#include "lexerDef.h"

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

char nextChar(FILE* fp, char *twinBuff, int *fwdPtr){
    if(*fwdPtr==BUFFER_SIZE-1){
        if(feof(fp))
            twinBuff[BUFFER_SIZE] = '\0';
        else {
            int bytesRead = fread(twinBuff+BUFFER_SIZE, sizeof(char), BUFFER_SIZE, fp);
            if(bytesRead<BUFFER_SIZE*sizeof(char)) twinBuff[BUFFER_SIZE+bytesRead]='\0';
        }
    }
    
    else if(*fwdPtr==(2*BUFFER_SIZE-1)){
        if(feof(fp))
            twinBuff[0] = '\0';
        else{
            int bytesRead = fread(twinBuff, sizeof(char), BUFFER_SIZE, fp);
            if(bytesRead<BUFFER_SIZE*sizeof(char)) twinBuff[bytesRead]='\0';  //Assuming that newline character will always be present at end
        }
    }
    
    *fwdPtr = (*fwdPtr + 1) % (2*BUFFER_SIZE);
    return twinBuff[*fwdPtr];
}

tokenInfo* getNextToken(FILE* fp, char *twinBuff, int *fwdPtr, int *lineNumber){
    tokenInfo* tokenNode;
    int beginPtr = (*fwdPtr + 1) % (2*BUFFER_SIZE);
    int state=0;
    char ch;

    for(; true; ){
        switch(state){
            case 0: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='\n') ++(*lineNumber);
                else if(isdigit(ch)) state=5;
                else if(ch=='%') state=65;
                else if(ch=='>') state=56;
                else if(ch=='<') state=53;
                else if(ch=='=') state=68;
                else if(ch=='_') state=69;
                else if(ch=='/') state=45;
                else if(ch=='@') state=73;
                else if(ch=='*') state=44;
                else if(ch=='#') state=75;
                else if(ch=='[') state=23;
                else if(ch==']') state=24;
                else if(ch==',') state=29;
                else if(ch==':') state=31;
                else if(ch==';') state=30;
                else if(ch=='+') state=42;
                else if(ch==')') state=35;
                else if(ch=='-') state=43;
                else if(ch=='(') state=34;
                else if(ch=='&') state=76;
                else if(ch=='~') state=52;
                else if(ch=='!') state=78;
                else if(ch=='a' || (ch>'d' && ch<='z')) state=3;
                else if(ch>='b' && ch<='d') state=79;
                else if(ch=='.') state=59;
                else if(ch=='\0'){
                    char lx[1] = {ch};
                    SymbolTableEntry* endSte = createToken(lx, DOLLAR, 0);
                    tokenInfo* endTok = createNewNode(endSte, *lineNumber);
                    return endTok;
                }
                else{
                    char lx[1] = {ch};
                    SymbolTableEntry* errSte = createToken(lx, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }
                break;

            case 1: 
                break;

            case 2: 
                break;

            case 3: 
                break;

            case 4: 
                break;

            case 5: 
                break;

            case 6: 
                break;

            case 7: 
                break;

            case 8: 
                break;

            case 9: 
                break;

            case 10: 
                break;

            case 11: 
                break;

            case 12: 
                break;

            case 13: 
                break;

            case 14: 
                break;

            case 15: 
                break;

            case 16: 
                break;

            case 17: 
                break;

            case 18: 
                break;

            case 19: 
                break;

            case 20: 
                break;

            case 21: 
                break;

            case 22: 
                break;

            case 23: 
                break;

            case 24: 
                break;

            case 25: 
                break;

            case 26: 
                break;

            case 27: 
                break;

            case 28: 
                break;

            case 29: 
                break;

            case 30: 
                break;

            case 31: 
                break;

            case 32: 
                break;

            case 33: 
                break;

            case 34: 
                break;

            case 35: 
                break;

            case 36: 
                break;

            case 37: 
                break;

            case 38: 
                break;

            case 39: 
                break;

            case 40: 
                break;

            case 41: 
                break;

            case 42: 
                break;

            case 43: 
                break;

            case 44: 
                break;

            case 45: 
                break;

            case 46: 
                break;

            case 47: 
                break;

            case 48: 
                break;

            case 49: 
                break;

            case 50: 
                break;

            case 51: 
                break;

            case 52: 
                break;

            case 53: 
                break;

            case 54: 
                break;

            case 55: 
                break;

            case 56: 
                break;

            case 57: 
                break;

            case 58: 
                break;

            case 59: 
                break;

            case 60: 
                break;

            case 61: 
                break;

            case 62: 
                break;

            case 63: 
                break;

            case 64: 
                break;

            case 65: 
                break;

            case 66: 
                break;

            case 67: 
                break;

            case 68: 
                break;

            case 69: 
                break;

            case 70: 
                break;

            case 71: 
                break;

            case 72: 
                break;

            case 73: 
                break;

            case 74: 
                break;

            case 75: 
                break;

            case 76: 
                break;

            case 77: 
                break;

            case 78: 
                break;

            case 79: 
                break;

            case 80: 
                break;

            case 81: 
                break;

            default:
                printf("Invalid state in lexer/dfa\n");
                return NULL;
                break;
        }
    }
}

void initializeKeywordsLookup(Trie* keywordsLookup){
    insertKeyword(keywordsLookup, "with", WITH);
    insertKeyword(keywordsLookup, "parameters", PARAMETERS);
    insertKeyword(keywordsLookup, "end", END);
    insertKeyword(keywordsLookup, "while", WHILE);
    insertKeyword(keywordsLookup, "union", UNION);
    insertKeyword(keywordsLookup, "endunion", ENDUNION);
    insertKeyword(keywordsLookup, "definetype", DEFINETYPE);
    insertKeyword(keywordsLookup, "as", AS);
    insertKeyword(keywordsLookup, "type", TYPE);
    insertKeyword(keywordsLookup, "global", GLOBAL);
    insertKeyword(keywordsLookup, "parameter", PARAMETER);
    insertKeyword(keywordsLookup, "list", LIST);
    insertKeyword(keywordsLookup, "input", INPUT);
    insertKeyword(keywordsLookup, "output", OUTPUT);
    insertKeyword(keywordsLookup, "int", INT);
    insertKeyword(keywordsLookup, "real", REAL);
    insertKeyword(keywordsLookup, "endwhile", ENDWHILE);
    insertKeyword(keywordsLookup, "if", IF);
    insertKeyword(keywordsLookup, "then", THEN);
    insertKeyword(keywordsLookup, "endif", ENDIF);
    insertKeyword(keywordsLookup, "read", READ);
    insertKeyword(keywordsLookup, "write", WRITE);
    insertKeyword(keywordsLookup, "return", RETURN);
    insertKeyword(keywordsLookup, "call", CALL);
    insertKeyword(keywordsLookup, "record", RECORD);
    insertKeyword(keywordsLookup, "endrecord", ENDRECORD);
    insertKeyword(keywordsLookup, "else", ELSE);
}

linkedList* getAllTokens(FILE* fp){
    char twinBuffer[BUFFER_SIZE*2];
    int fwdPtr = 2*BUFFER_SIZE-1;
    int lineNumber=1;

    Trie* keywordsLookup=createTrie();
    initializeKeywordsLookup(keywordsLookup);
    
    SymbolTable* symbolTable = createSymbolTable();

    linkedList* tokensList = createNewList();


    for(; true; ){
        tokenInfo* tkInfo = getNextToken(fp, twinBuffer, &fwdPtr, &lineNumber);
        if(!tkInfo){
            printf("No token retrieved\n");
            break;
        }
        insertLLNode(tokensList, tkInfo);

        if(tkInfo->STE->tokenType==DOLLAR) break;
    }
    return tokensList;
}




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


int main(){
    printf("hello");
}