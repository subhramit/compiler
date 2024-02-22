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

void tokenizeLexeme(int beginPtr, int* fwdPtr, char* lexeme, char* twinBuff){
    if(*fwdPtr >= beginPtr){
        int i;
        for(i=0; i<=(*fwdPtr-beginPtr); i++) 
            lexeme[i] = twinBuff[beginPtr+i];
        lexeme[i] = '\0';
    }
    else{
        int i;
        for(i=0; i<=(*fwdPtr+2*BUFFER_SIZE-beginPtr); i++) 
            lexeme[i] = twinBuff[(beginPtr+i)%(2*BUFFER_SIZE)];
        lexeme[i] = '\0';
    }
}

tokenInfo* getNextToken(FILE* fp, char *twinBuff, int *fwdPtr, int *lineNumber, Trie* keywordsLookup){
    tokenInfo* tokenNode;
    int beginPtr = (*fwdPtr + 1) % (2*BUFFER_SIZE);
    int state=0;
    char ch;
    char lexeme[BUFFER_SIZE];

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
                    lexeme[0] = ch;
                    SymbolTableEntry* endSte = createToken(lexeme, DOLLAR, 0);
                    tokenInfo* endTok = createNewNode(endSte, *lineNumber);
                    return endTok;
                }
                else{
                    lexeme[0] = ch; lexeme[1] = '\0';
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }
                break;

            case 1: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* asgnSte = createToken(lexeme, ASSIGNOP, 0);
                tokenInfo* asgnTok = createNewNode(asgnSte, *lineNumber);
                return asgnTok;
                break;

            case 2: 
                state=0;
                break;

            case 3: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(!islower(ch)){    // Retract once and tokenize
                    --(*fwdPtr); 

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);

                    Token tki = searchKeyword(keywordsLookup, lexeme);
                    if(tki==NOT_FOUND){
                        SymbolTableEntry* fidSte = createToken(lexeme, FIELDID, 0);
                        tokenInfo* fidTok = createNewNode(fidSte, *lineNumber);
                        return  fidTok;
                    }
                    else{
                        SymbolTableEntry* kwSte = createToken(lexeme, tki, 0);
                        tokenInfo* kwTok = createNewNode(kwSte, *lineNumber);
                        return kwTok;
                    }
                    
                }

                break;

            case 4: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch<'2' || ch>'7'){   // Retract once and tokenize
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    int l = strlen(lexeme);
                    if(l>20){
                        SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                        return errTok;
                    }
                    SymbolTableEntry* idSte = createToken(lexeme, ID, 0);
                    tokenInfo* idTok = createNewNode(idSte, *lineNumber);
                    return idTok;
                }

                break;

            case 5: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='.'){
                    state=60;
                }
                else if(!isdigit(ch)){
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    double val=0;
                    for(int i=0; lexeme[i]; i++){
                        val *= 10;
                        val += lexeme[i]-'0';
                    }
                    SymbolTableEntry* numSte = createToken(lexeme, NUM, val);
                    tokenInfo* numTok = createNewNode(numSte, *lineNumber);
                    return numTok;
                }

                break;

            case 6: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='E') state=62;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    double val=0; int i;
                    for(i=0; lexeme[i]!='.'; i++){
                        val *= 10;
                        val += lexeme[i] - '0';
                    }
                    val += (lexeme[i+1]-'0')/10.0 + (lexeme[i+2]-'0')/100.0;
                    SymbolTableEntry* rnumSte = createToken(lexeme, RNUM, val);
                    tokenInfo* rnumTok = createNewNode(rnumSte, *lineNumber);
                    return rnumTok;
                }   

                break;

            case 7: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                double val=0; int i;
                for(i=0; lexeme[i]!='.'; i++){
                    val *= 10;
                    val += lexeme[i] - '0';
                }
                val += (lexeme[i+1]-'0')/10.0 + (lexeme[i+2]-'0')/100.0;
                int exp=0;
                i+=4;
                if(isdigit(lexeme[i]))
                    exp = lexeme[i]*10 + lexeme[i+1];
                else 
                    exp = lexeme[i+1]*10 + lexeme[i+2];
                
                if(lexeme[i]=='-') exp = -exp;
                val *= pow(10, exp);

                SymbolTableEntry* rnumSte = createToken(lexeme, RNUM, val);
                tokenInfo* rnumTok = createNewNode(rnumSte, *lineNumber);
                return rnumTok;

                break;

            case 8: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=81;
                else if(!isalpha(ch)){
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* fnidSte = createToken(lexeme, FUNID, 0);
                    tokenInfo* fnidTok = createNewNode(fnidSte, *lineNumber);
                    return fnidTok;
                }

                break;

            case 9: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(!islower(ch)){
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* ridSte = createToken(lexeme, RUID, 0);
                    tokenInfo* ridTok = createNewNode(ridSte, *lineNumber);
                    return ridTok;
                }

                break;

            case 19: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isalpha(ch)) state=8;
                else if(isdigit(ch)) state=81;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* mSte = createToken(lexeme, MAIN, 0);
                    tokenInfo* mTok = createNewNode(mSte, *lineNumber);
                    return mTok;
                }

                break;

            case 23: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* sqlSte = createToken(lexeme, SQL, 0);
                tokenInfo* sqlTok = createNewNode(sqlSte, *lineNumber);
                return sqlTok;

                break;

            case 24: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* sqrSte = createToken(lexeme, SQR, 0);
                tokenInfo* sqrTok = createNewNode(sqrSte, *lineNumber);
                return sqrTok;

                break;

            case 29: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* cmSte = createToken(lexeme, COMMA, 0);
                tokenInfo* cmTok = createNewNode(cmSte, *lineNumber);
                return cmTok;

                break;

            case 30: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* smcSte = createToken(lexeme, SEM, 0);
                tokenInfo* smcTok = createNewNode(smcSte, *lineNumber);
                return smcTok;

                break;

            case 31: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* clnSte = createToken(lexeme, COLON, 0);
                tokenInfo* clnTok = createNewNode(clnSte, *lineNumber);
                return clnTok;

                break;

            case 34: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* opSte = createToken(lexeme, OP, 0);
                tokenInfo* opTok = createNewNode(opSte, *lineNumber);
                return opTok;

                break;

            case 35: 
                
                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* clSte = createToken(lexeme, CL, 0);
                tokenInfo* clTok = createNewNode(clSte, *lineNumber);
                return clTok;

                break;

            case 42: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* plSte = createToken(lexeme, PLUS, 0);
                tokenInfo* plTok = createNewNode(plSte, *lineNumber);
                return plTok;

                break;

            case 43: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* mnsSte = createToken(lexeme, MINUS, 0);
                tokenInfo* mnsTok = createNewNode(mnsSte, *lineNumber);
                return mnsTok;

                break;

            case 44: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* mlSte = createToken(lexeme, MUL, 0);
                tokenInfo* mlTok = createNewNode(mlSte, *lineNumber);
                return mlTok;

                break;

            case 45: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* dvSte = createToken(lexeme, DIV, 0);
                tokenInfo* dvTok = createNewNode(dvSte, *lineNumber);
                return dvTok;

                break;

            case 50: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* andSte = createToken(lexeme, AND, 0);
                tokenInfo* andTok = createNewNode(andSte, *lineNumber);
                return andTok;

                break;

            case 51: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* orSte = createToken(lexeme, OR, 0);
                tokenInfo* orTok = createNewNode(orSte, *lineNumber);
                return orTok;

                break;

            case 52: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* notSte = createToken(lexeme, NOT, 0);
                tokenInfo* notTok = createNewNode(notSte, *lineNumber);
                return notTok;

                break;

            case 53: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='-') state=66;
                else if(ch=='=') state=54;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* ltSte = createToken(lexeme, LT, 0);
                    tokenInfo* ltTok = createNewNode(ltSte, *lineNumber);
                    return ltTok;
                }

                break;

            case 54: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* leSte = createToken(lexeme, LE, 0);
                tokenInfo* leTok = createNewNode(leSte, *lineNumber);
                return leTok;

                break;

            case 55: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* eqSte = createToken(lexeme, EQ, 0);
                tokenInfo* eqTok = createNewNode(eqSte, *lineNumber);
                return eqTok;

                break;

            case 56: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='=') state=57;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* gtSte = createToken(lexeme, GT, 0);
                    tokenInfo* gtTok = createNewNode(gtSte, *lineNumber);
                    return gtTok;
                }

                break;

            case 57: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* geSte = createToken(lexeme, GE, 0);
                tokenInfo* geTok = createNewNode(geSte, *lineNumber);
                return geTok;

                break;

            case 58: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* neSte = createToken(lexeme, NE, 0);
                tokenInfo* neTok = createNewNode(neSte, *lineNumber);
                return neTok;

                break;

            case 59: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* dotSte = createToken(lexeme, DOT, 0);
                tokenInfo* dotTok = createNewNode(dotSte, *lineNumber);
                return dotTok;

                break;

            case 60: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=61;

                else{   
                    *fwdPtr -=2;    // Double retract 

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    double val=0;
                    for(int i=0; lexeme[i]; i++){
                        val *= 10;
                        val += lexeme[i]-'0';
                    }
                    SymbolTableEntry* numSte = createToken(lexeme, NUM, val);
                    tokenInfo* numTok = createNewNode(numSte, *lineNumber);
                    return numTok;
                }

                break;

            case 61: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=6;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 62: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=64;
                else if(ch=='+' || ch=='-') state=63;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 63: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=64;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 64: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=7;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 65: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='\n'){
                    state=2;
                    ++(*lineNumber);
                }

                break;

            case 66: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='-') state=67;
                else{
                    *fwdPtr -=2;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* ltSte = createToken(lexeme, LT, 0);
                    tokenInfo* ltTok = createNewNode(ltSte, *lineNumber);
                    return ltTok;
                }

                break;

            case 67: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='-') state=1;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 68: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='=') state=55;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 69: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='m') state=70;
                else if(isalpha(ch)) state=8;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 70: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=81;
                else if(ch=='a') state=71;
                else if(isalpha(ch)) state=8;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    int l = strlen(lexeme);
                    if(l>30){
                        SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                        return errTok;
                    }
                    SymbolTableEntry* fnidSte = createToken(lexeme, FUNID, 0);
                    tokenInfo* fnidTok = createNewNode(fnidSte, *lineNumber);
                    return fnidTok;
                }

                break;

            case 71: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=81;
                else if(ch=='i') state=72;
                else if(isalpha(ch)) state=8;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    int l = strlen(lexeme);
                    if(l>30){
                        SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                        return errTok;
                    }
                    SymbolTableEntry* fnidSte = createToken(lexeme, FUNID, 0);
                    tokenInfo* fnidTok = createNewNode(fnidSte, *lineNumber);
                    return fnidTok;
                }

                break;

            case 72: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=81;
                else if(ch=='n') state=19;
                else if(isalpha(ch)) state=8;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    int l = strlen(lexeme);
                    if(l>30){
                        SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                        return errTok;
                    }
                    SymbolTableEntry* fnidSte = createToken(lexeme, FUNID, 0);
                    tokenInfo* fnidTok = createNewNode(fnidSte, *lineNumber);
                    return fnidTok;
                }

                break;

            case 73: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='@') state=74;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 74: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='@') state=51;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 75: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(islower(ch)) state=9;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 76: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='&') state=77;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 77: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='&') state=78;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 78: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='=') state=58;
                else{
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 79: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(islower(ch)) state=3;
                else if(ch>='2' && ch<='7') state=80;
                else{
                    --(*fwdPtr); 

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);

                    Token tki = searchKeyword(keywordsLookup, lexeme);
                    if(tki==NOT_FOUND){
                        SymbolTableEntry* fidSte = createToken(lexeme, FIELDID, 0);
                        tokenInfo* fidTok = createNewNode(fidSte, *lineNumber);
                        return  fidTok;
                    }
                    else{
                        SymbolTableEntry* kwSte = createToken(lexeme, tki, 0);
                        tokenInfo* kwTok = createNewNode(kwSte, *lineNumber);
                        return kwTok;
                    }
                }

                break;

            case 80: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch>='2' && ch<='7') state=4;
                else if(ch<'b' || ch>'d'){
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    int l = strlen(lexeme);
                    if(l>20){
                        SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                        return errTok;
                    }
                    SymbolTableEntry* idSte = createToken(lexeme, ID, 0);
                    tokenInfo* idTok = createNewNode(idSte, *lineNumber);
                    return idTok;
                }

                break;

            case 81: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(!isdigit(ch)){
                    --(*fwdPtr);

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* fnidSte = createToken(lexeme, FUNID, 0);
                    tokenInfo* fnidTok = createNewNode(fnidSte, *lineNumber);
                    return fnidTok;
                }

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
        tokenInfo* tkInfo = getNextToken(fp, twinBuffer, &fwdPtr, &lineNumber, keywordsLookup);
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