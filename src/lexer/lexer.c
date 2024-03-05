/*
#                    GROUP - 8
# 2020B1A70630P                       Aditya Thakur
# 2021A7PS2001P                       Amal Sayeed
# 2021A7PS2005P                       Ohiduz Zaman
# 2021A7PS2682P                       Priyansh Patel
# 2021A7PS2002P                       Rachoita Das
# 2020B1A70611P                       Subhramit Basu Bhowmick
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lexer.h"
#include "lexerDef.h"

char* tokenToString[TK_NOT_FOUND];
bool specialThing=false;
bool tkSInit=false;


TrieNode* createTrieNode() {
    /*
        Function to create a new Trie node; The trie data structure will be used for fast and efficient lookup of keywords
    */ 
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

Trie* createTrie(){
    /*
        Function to create a new Trie and its root node
    */ 
    Trie* newTrie = (Trie*) malloc(sizeof(Trie));
    if(newTrie){
        newTrie->root = createTrieNode();
    }
    else{
        printf("Could not allocate memory for creating trie\n");
    }
    return newTrie;
}

void insertKeyword(Trie* myTrie, const char* word, Token tkType) {
    /*
        Insert a keyword into the Trie
    */
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

Token searchKeyword(Trie* myTrie, const char* word) {
    /*
        Searches for a given word in the Trie. Returns the type of token found or NT_NOT_FOUND if word doesn't exist
    */
    TrieNode* currentNode = myTrie->root;
    for (int i = 0; word[i] != '\0'; i++) {
        int index = word[i] - 'a';
        if (!(currentNode->children[index])) {
            return TK_NOT_FOUND; // Word not found
        }
        currentNode = currentNode->children[index];
    }
    if(currentNode && currentNode->isEndOfWord)
        return currentNode->tokenType;
    return TK_NOT_FOUND;
}

SymbolTable* createSymbolTable(){
    /*
        Create a new Symbol Table and allocate some initial capacity.
        We will resize as needed.
    */
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


void insertToken(SymbolTable* ST, SymbolTableEntry* stEntry){
    /*
        After tokenizing, insert the token and other details as an entry into the Symbol Table.
    */
    if(ST->size == ST->capacity){   
        // If size of table reaches the current capacity, resize it to double its capacity.
        ST->capacity *= 2;
        ST->table = (SymbolTableEntry**) realloc(ST->table , ST->capacity*sizeof(SymbolTableEntry*));
        if(!(ST->table)){
            printf("Could not allocate memory for resizing Symbol Table\n");
            return;
        }
    }
    ST->table[(ST->size)++] = stEntry;  // Add the new entry at the end of the table and increment size.
}

SymbolTableEntry* createToken(char* lxm, Token tkType, double valNum){
    /*
        // Create a token / an entry in the Symbol Table from lexeme and type information.
    */
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

SymbolTableEntry* searchToken(SymbolTable* ST, char* lxm){
    /*
        Search for a lexeme in the Symbol Table
    */
    for(int i=0; i<ST->size; ++i)
        if(!strcmp(lxm, ST->table[i]->lexeme)) 
            return ST->table[i];
    return NULL;
}

linkedList* createNewList(){
    // Create a linked list for storing tokens tokenized by the Lexer
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
    // Create a linkedList node for storing token information from the symbol table combined with line number
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
    // Inserts a node (corresponding to each token information) into the list of tokens
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

char nextChar(FILE* fp, char *twinBuff, int *fwdPtr){
    /*
        NOTE: We are NOT fetching character by character from the file.
        This function simply advances the forward pointer within the twin buffer and returns the character at the 
        position of the forward pointer. If the forward pointer reaches the end of either one of the twin buffers,
        then the other buffer is populated by reading a stream of characters  at once from the file.
    */
    if(*fwdPtr==BUFFER_SIZE-1 && !specialThing){ 
        // If forward pointer is in end of first buffer
        if(feof(fp))
            twinBuff[BUFFER_SIZE] = '\0';
        else {
            int bytesRead = fread(twinBuff+BUFFER_SIZE, sizeof(char), BUFFER_SIZE, fp);
            if(bytesRead<BUFFER_SIZE*sizeof(char)) twinBuff[BUFFER_SIZE+bytesRead]='\0';
        }
    }
    
    else if(*fwdPtr==(2*BUFFER_SIZE-1) && !specialThing){
        // If forward pointer is in end of second buffer
        if(feof(fp))
            twinBuff[0] = '\0';
        else{
            int bytesRead = fread(twinBuff, sizeof(char), BUFFER_SIZE, fp);
            if(bytesRead<BUFFER_SIZE*sizeof(char)) twinBuff[bytesRead]='\0';  
        }
    }
    if(specialThing) specialThing=false;
    
    *fwdPtr = (*fwdPtr + 1) % (2*BUFFER_SIZE);
    return twinBuff[*fwdPtr];
}

void tokenizeLexeme(int beginPtr, int* fwdPtr, char* lexeme, char* twinBuff){
    // Copies the string corresponding to the characters between begin and forward pointer into the variable lexeme
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

int getLength(int begPtr, int fwdPtr){
    // Returns length of string corresponding to characters between begin and forward pointers
    if(fwdPtr>=begPtr) return  (fwdPtr-begPtr+1);
    else return (2*BUFFER_SIZE+fwdPtr-begPtr+1);
}

tokenInfo* getNextToken(FILE* fp, char *twinBuff, int *fwdPtr, int *lineNumber, Trie* keywordsLookup, SymbolTable* symbolTable){
    /*
        This function is the DFA for pattern matching of the lexer. The cases in the switch correspond to states. 
        Based on characters read, takes appropriate actions and changes states following the transition rules of the DFA
    */
    tokenInfo* tokenNode;
    int beginPtr = (*fwdPtr + 1) % (2*BUFFER_SIZE);
    int state=0;
    char ch;
    char lexeme[BUFFER_SIZE];

    for(; true; ){
        switch(state){
            case 0: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='\n') {++(*lineNumber); beginPtr++; beginPtr %= 2*BUFFER_SIZE;}
                else if(ch==' ' || ch=='\t' || ch=='\r') {beginPtr++; beginPtr %= 2*BUFFER_SIZE;}
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
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }
                break;

            case 1: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* asgnSte = searchToken(symbolTable, lexeme);
                if(!asgnSte){
                    asgnSte = createToken(lexeme, ASSIGNOP, 0);
                    insertToken(symbolTable, asgnSte);
                }
                tokenInfo* asgnTok = createNewNode(asgnSte, *lineNumber);
                return asgnTok;
                break;

            case 2: 
                // tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                // lexeme[1]='\0';
                // SymbolTableEntry* cmntSte = searchToken(symbolTable, lexeme);
                // if(!cmntSte){
                //     cmntSte = createToken(lexeme, COMMENT, 0);
                //     insertToken(symbolTable, cmntSte);
                // }
                // tokenInfo* cmntTok = createNewNode(cmntSte, *lineNumber);
                // ++(*lineNumber);
                // return cmntTok;
                break;

            case 3: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(!islower(ch)){    // Retract once and tokenize
                    --(*fwdPtr); if(*fwdPtr<0){ *fwdPtr += 2*BUFFER_SIZE; }
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);

                    SymbolTableEntry* ste = searchToken(symbolTable, lexeme);
                    if(!ste){
                        Token tki = searchKeyword(keywordsLookup, lexeme);
                        if(tki==TK_NOT_FOUND){
                            SymbolTableEntry* fidSte = createToken(lexeme, FIELDID, 0);
                            insertToken(symbolTable, fidSte);
                            tokenInfo* fidTok = createNewNode(fidSte, *lineNumber);
                            return  fidTok;
                        }
                        else{
                            SymbolTableEntry* kwSte = createToken(lexeme, tki, 0);
                            insertToken(symbolTable, kwSte);
                            tokenInfo* kwTok = createNewNode(kwSte, *lineNumber);
                            return kwTok;
                        }
                    }
                    else{
                        tokenInfo* Tok = createNewNode(ste, *lineNumber);
                        return Tok;
                    }
                    
                }

                break;

            case 4: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch<'2' || ch>'7'){   // Retract once and tokenize
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    // int l = strlen(lexeme);
                    // if(l>20){
                    //     SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    //     if(!errSte){
                    //         errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    //         insertToken(symbolTable, errSte);
                    //     }
                    //     tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    //     return errTok;
                    // }
                    SymbolTableEntry* idSte = searchToken(symbolTable, lexeme);
                    if(!idSte){
                        idSte = createToken(lexeme, ID, 0);
                        insertToken(symbolTable, idSte);
                    }
                    tokenInfo* idTok = createNewNode(idSte, *lineNumber);
                    return idTok;
                }
                if(getLength(beginPtr, *fwdPtr)>20){
                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    lexeme[20]='.'; lexeme[21]='.'; lexeme[22]='.'; lexeme[23]='\0';
                    SymbolTableEntry* idLSte = searchToken(symbolTable, lexeme);
                    if(!idLSte){
                        idLSte = createToken(lexeme, ID_LENGTH_EXC, 0);
                        insertToken(symbolTable, idLSte);
                    }
                    tokenInfo* idExTok = createNewNode(idLSte, *lineNumber);
                    ch = nextChar(fp, twinBuff, fwdPtr);
                    for(; ch>='2' && ch<='7'; ch=nextChar(fp, twinBuff, fwdPtr));
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;
                    return idExTok;
                }

                break;

            case 5: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='.'){
                    state=60;
                }
                else if(!isdigit(ch)){
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* numSte = searchToken(symbolTable, lexeme);
                    if(numSte){
                        tokenInfo* numTok = createNewNode(numSte, *lineNumber);
                        return numTok;
                    }
                    double val=0;
                    for(int i=0; lexeme[i]; i++){
                        val *= 10;
                        val += lexeme[i]-'0';
                    }
                    numSte = createToken(lexeme, NUM, val);
                    insertToken(symbolTable, numSte);
                    tokenInfo* numTok = createNewNode(numSte, *lineNumber);
                    return numTok;
                }

                break;

            case 6: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='E') state=62;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* rnumSte = searchToken(symbolTable, lexeme);
                    if(rnumSte){
                        tokenInfo* rnumTok = createNewNode(rnumSte, *lineNumber);
                        return rnumTok;
                    }
                    double val=0; int i;
                    for(i=0; lexeme[i]!='.'; i++){
                        val *= 10;
                        val += lexeme[i] - '0';
                    }
                    val += (lexeme[i+1]-'0')/10.0 + (lexeme[i+2]-'0')/100.0;
                    rnumSte = createToken(lexeme, RNUM, val);
                    insertToken(symbolTable, rnumSte);
                    tokenInfo* rnumTok = createNewNode(rnumSte, *lineNumber);
                    return rnumTok;
                }   

                break;

            case 7: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* rnumSte = searchToken(symbolTable, lexeme);
                if(rnumSte){
                    tokenInfo* rnumTok = createNewNode(rnumSte, *lineNumber);
                    return rnumTok;
                }
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

                rnumSte = createToken(lexeme, RNUM, val);
                insertToken(symbolTable, rnumSte);
                tokenInfo* rnumTok = createNewNode(rnumSte, *lineNumber);
                return rnumTok;

                break;

            case 8: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=81;
                else if(!isalpha(ch)){
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* cmnSte = searchToken(symbolTable, lexeme);
                    if(cmnSte){
                        tokenInfo* cmnTok = createNewNode(cmnSte, *lineNumber);
                        return cmnTok;
                    }
                    // int l = strlen(lexeme);
                    // if(l>30){
                    //     SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    //     insertToken(symbolTable, errSte);
                    //     tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    //     return errTok;
                    // }
                    SymbolTableEntry* fnidSte = createToken(lexeme, FUNID, 0);
                    insertToken(symbolTable, fnidSte);
                    tokenInfo* fnidTok = createNewNode(fnidSte, *lineNumber);
                    return fnidTok;
                }
                if(getLength(beginPtr, *fwdPtr)>30){
                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    lexeme[30]='.'; lexeme[31]='.'; lexeme[32]='.'; lexeme[33]='\0';
                    SymbolTableEntry* fnLSte = searchToken(symbolTable, lexeme);
                    if(!fnLSte){
                        fnLSte = createToken(lexeme, FUN_LENGTH_EXC, 0);
                        insertToken(symbolTable, fnLSte);
                    }
                    tokenInfo* fnExTok = createNewNode(fnLSte, *lineNumber);
                    ch = nextChar(fp, twinBuff, fwdPtr);
                    for(; isalpha(ch); ch=nextChar(fp, twinBuff, fwdPtr));
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;
                    if(isdigit(ch)){
                        state=81; break;
                    }
                    return fnExTok;
                }

                break;

            case 9: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(!islower(ch)){
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* ridSte = searchToken(symbolTable, lexeme);
                    if(!ridSte){
                        ridSte = createToken(lexeme, RUID, 0);
                        insertToken(symbolTable, ridSte);
                    }
                    tokenInfo* ridTok = createNewNode(ridSte, *lineNumber);
                    return ridTok;
                }

                break;

            case 19: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isalpha(ch)) state=8;
                else if(isdigit(ch)) state=81;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* mSte = searchToken(symbolTable, lexeme);
                    if(!mSte){
                        mSte = createToken(lexeme, MAIN, 0);
                        insertToken(symbolTable, mSte);
                    }
                    tokenInfo* mTok = createNewNode(mSte, *lineNumber);
                    return mTok;
                }

                break;

            case 23: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* sqlSte = searchToken(symbolTable, lexeme);
                if(!sqlSte){
                    sqlSte = createToken(lexeme, SQL, 0);
                    insertToken(symbolTable, sqlSte);
                }
                tokenInfo* sqlTok = createNewNode(sqlSte, *lineNumber);
                return sqlTok;

                break;

            case 24: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* sqrSte = searchToken(symbolTable, lexeme);
                if(!sqrSte){
                    sqrSte = createToken(lexeme, SQR, 0);
                    insertToken(symbolTable, sqrSte);
                }
                tokenInfo* sqrTok = createNewNode(sqrSte, *lineNumber);
                return sqrTok;

                break;

            case 29: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* cmSte = searchToken(symbolTable, lexeme);
                if(!cmSte){
                    cmSte = createToken(lexeme, COMMA, 0);
                    insertToken(symbolTable, cmSte);
                }
                tokenInfo* cmTok = createNewNode(cmSte, *lineNumber);
                return cmTok;

                break;

            case 30: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* smcSte = searchToken(symbolTable, lexeme);
                if(!smcSte){
                    smcSte = createToken(lexeme, SEM, 0);
                    insertToken(symbolTable, smcSte);
                }
                tokenInfo* smcTok = createNewNode(smcSte, *lineNumber);
                return smcTok;

                break;

            case 31: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* clnSte = searchToken(symbolTable, lexeme);
                if(!clnSte){
                    clnSte = createToken(lexeme, COLON, 0);
                    insertToken(symbolTable, clnSte);
                }
                tokenInfo* clnTok = createNewNode(clnSte, *lineNumber);
                return clnTok;

                break;

            case 34: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* opSte = searchToken(symbolTable, lexeme);
                if(!opSte){
                    opSte = createToken(lexeme, OP, 0);
                    insertToken(symbolTable, opSte);
                }
                tokenInfo* opTok = createNewNode(opSte, *lineNumber);
                return opTok;

                break;

            case 35: 
                
                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* clSte = searchToken(symbolTable, lexeme);
                if(!clSte){
                    clSte = createToken(lexeme, CL, 0);
                    insertToken(symbolTable, clSte);
                }
                tokenInfo* clTok = createNewNode(clSte, *lineNumber);
                return clTok;

                break;

            case 42: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* plSte = searchToken(symbolTable, lexeme);
                if(!plSte){
                    plSte = createToken(lexeme, PLUS, 0);
                    insertToken(symbolTable, plSte);
                }
                tokenInfo* plTok = createNewNode(plSte, *lineNumber);
                return plTok;

                break;

            case 43: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* mnsSte = searchToken(symbolTable, lexeme);
                if(!mnsSte){
                    mnsSte = createToken(lexeme, MINUS, 0);
                    insertToken(symbolTable, mnsSte);
                }
                tokenInfo* mnsTok = createNewNode(mnsSte, *lineNumber);
                return mnsTok;

                break;

            case 44: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* mlSte = searchToken(symbolTable, lexeme);
                if(!mlSte){
                    mlSte = createToken(lexeme, MUL, 0);
                    insertToken(symbolTable, mlSte);
                }
                tokenInfo* mlTok = createNewNode(mlSte, *lineNumber);
                return mlTok;

                break;

            case 45: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* dvSte = searchToken(symbolTable, lexeme);
                if(!dvSte){
                    dvSte = createToken(lexeme, DIV, 0);
                    insertToken(symbolTable, dvSte);
                }
                tokenInfo* dvTok = createNewNode(dvSte, *lineNumber);
                return dvTok;

                break;

            case 50: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* andSte = searchToken(symbolTable, lexeme);
                if(!andSte){
                    andSte = createToken(lexeme, AND, 0);
                    insertToken(symbolTable, andSte);
                }
                tokenInfo* andTok = createNewNode(andSte, *lineNumber);
                return andTok;

                break;

            case 51: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* orSte = searchToken(symbolTable, lexeme);
                if(!orSte){
                    orSte = createToken(lexeme, OR, 0);
                    insertToken(symbolTable, orSte);
                }
                tokenInfo* orTok = createNewNode(orSte, *lineNumber);
                return orTok;

                break;

            case 52: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* notSte = searchToken(symbolTable, lexeme);
                if(!notSte){
                    notSte = createToken(lexeme, NOT, 0);
                    insertToken(symbolTable, notSte);
                }
                tokenInfo* notTok = createNewNode(notSte, *lineNumber);
                return notTok;

                break;

            case 53: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='-') state=66;
                else if(ch=='=') state=54;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* ltSte = searchToken(symbolTable, lexeme);
                    if(!ltSte){
                        ltSte = createToken(lexeme, LT, 0);
                        insertToken(symbolTable, ltSte);
                    }
                    tokenInfo* ltTok = createNewNode(ltSte, *lineNumber);
                    return ltTok;
                }

                break;

            case 54: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* leSte = searchToken(symbolTable, lexeme);
                if(!leSte){
                    leSte = createToken(lexeme, LE, 0);
                    insertToken(symbolTable, leSte);
                }
                tokenInfo* leTok = createNewNode(leSte, *lineNumber);
                return leTok;

                break;

            case 55: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* eqSte = searchToken(symbolTable, lexeme);
                if(!eqSte){
                    eqSte = createToken(lexeme, EQ, 0);
                    insertToken(symbolTable, eqSte);
                }
                tokenInfo* eqTok = createNewNode(eqSte, *lineNumber);
                return eqTok;

                break;

            case 56: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='=') state=57;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* gtSte = searchToken(symbolTable, lexeme);
                    if(!gtSte){
                        gtSte = createToken(lexeme, GT, 0);
                        insertToken(symbolTable, gtSte);
                    }
                    tokenInfo* gtTok = createNewNode(gtSte, *lineNumber);
                    return gtTok;
                }

                break;

            case 57: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* geSte = searchToken(symbolTable, lexeme);
                if(!geSte){
                    geSte = createToken(lexeme, GE, 0);
                    insertToken(symbolTable, geSte);
                }
                tokenInfo* geTok = createNewNode(geSte, *lineNumber);
                return geTok;

                break;

            case 58: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* neSte = searchToken(symbolTable, lexeme);
                if(!neSte){
                    neSte = createToken(lexeme, NE, 0);
                    insertToken(symbolTable, neSte);
                }
                tokenInfo* neTok = createNewNode(neSte, *lineNumber);
                return neTok;

                break;

            case 59: 

                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* dotSte = searchToken(symbolTable, lexeme);
                if(!dotSte){
                    dotSte = createToken(lexeme, DOT, 0);
                    insertToken(symbolTable, dotSte);
                }
                tokenInfo* dotTok = createNewNode(dotSte, *lineNumber);
                return dotTok;

                break;

            case 60: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=61;

                else{   
                    *fwdPtr -=2; if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;  // Double retract 
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==BUFFER_SIZE-2 || *fwdPtr==2*BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-2) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* numSte = searchToken(symbolTable, lexeme);
                    if(numSte){
                        tokenInfo* numTok = createNewNode(numSte, *lineNumber);
                        return numTok;
                    }
                    double val=0;
                    for(int i=0; lexeme[i]; i++){
                        val *= 10;
                        val += lexeme[i]-'0';
                    }
                    numSte = createToken(lexeme, NUM, val);
                    insertToken(symbolTable, numSte);
                    tokenInfo* numTok = createNewNode(numSte, *lineNumber);
                    return numTok;
                }

                break;

            case 61: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=6;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 62: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=64;
                else if(ch=='+' || ch=='-') state=63;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 63: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=64;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 64: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(isdigit(ch)) state=7;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 65: 
                tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                SymbolTableEntry* cmntSte = searchToken(symbolTable, lexeme);
                if(!cmntSte){
                    cmntSte = createToken(lexeme, COMMENT, 0);
                    insertToken(symbolTable, cmntSte);
                }
                tokenInfo* cmntTok = createNewNode(cmntSte, *lineNumber);


                for(; (ch = nextChar(fp, twinBuff, fwdPtr))!='\n';);

                ++(*lineNumber);
                return cmntTok;
                break;

            case 66: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='-') state=67;
                else{
                    *fwdPtr -=2; if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==BUFFER_SIZE-2 || *fwdPtr==2*BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-2) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* ltSte = searchToken(symbolTable, lexeme);
                    if(!ltSte){
                        ltSte = createToken(lexeme, LT, 0);
                        insertToken(symbolTable, ltSte);
                    }
                    tokenInfo* ltTok = createNewNode(ltSte, *lineNumber);
                    return ltTok;
                }

                break;

            case 67: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='-') state=1;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 68: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='=') state=55;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 69: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='m') state=70;
                else if(isalpha(ch)) state=8;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
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
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* fnidSte = searchToken(symbolTable, lexeme);
                    if(!fnidSte){
                        fnidSte = createToken(lexeme, FUNID, 0);
                        insertToken(symbolTable, fnidSte);
                    }
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
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* fnidSte = searchToken(symbolTable, lexeme);
                    if(!fnidSte){
                        fnidSte = createToken(lexeme, FUNID, 0);
                        insertToken(symbolTable, fnidSte);
                    }
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
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* fnidSte = searchToken(symbolTable, lexeme);
                    if(!fnidSte){
                        fnidSte = createToken(lexeme, FUNID, 0);
                        insertToken(symbolTable, fnidSte);
                    }
                    tokenInfo* fnidTok = createNewNode(fnidSte, *lineNumber);
                    return fnidTok;
                }

                break;

            case 73: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='@') state=74;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 74: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='@') state=51;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 75: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(islower(ch)) state=9;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 76: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='&') state=77;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 77: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='&') state=50;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 78: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch=='=') state=58;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* errSte = searchToken(symbolTable, lexeme);
                    if(!errSte){
                        errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                        insertToken(symbolTable, errSte);
                    }
                    tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    return errTok;
                }

                break;

            case 79: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(islower(ch)) state=3;
                else if(ch>='2' && ch<='7') state=80;
                else{
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* cmnSte = searchToken(symbolTable, lexeme);
                    if(cmnSte){
                        tokenInfo* cmnTok = createNewNode(cmnSte, *lineNumber);
                        return  cmnTok;
                    }

                    Token tki = searchKeyword(keywordsLookup, lexeme);
                    if(tki==TK_NOT_FOUND){
                        SymbolTableEntry* fidSte = createToken(lexeme, FIELDID, 0);
                        insertToken(symbolTable, fidSte);
                        tokenInfo* fidTok = createNewNode(fidSte, *lineNumber);
                        return  fidTok;
                    }
                    else{
                        SymbolTableEntry* kwSte = createToken(lexeme, tki, 0);
                        insertToken(symbolTable, kwSte);
                        tokenInfo* kwTok = createNewNode(kwSte, *lineNumber);
                        return kwTok;
                    }
                }

                break;

            case 80: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(ch>='2' && ch<='7') state=4;
                else if(ch<'b' || ch>'d'){
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* cmnSte = searchToken(symbolTable, lexeme);
                    if(cmnSte){
                        tokenInfo* cmnTok = createNewNode(cmnSte, *lineNumber);
                        return cmnTok;
                    }
                    // int l = strlen(lexeme);
                    // if(l>20){
                    //     SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    //     insertToken(symbolTable, errSte);
                    //     tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    //     return errTok;
                    // }
                    SymbolTableEntry* idSte = createToken(lexeme, ID, 0);
                    insertToken(symbolTable, idSte);
                    tokenInfo* idTok = createNewNode(idSte, *lineNumber);
                    return idTok;
                }
                if(getLength(beginPtr, *fwdPtr)>20){
                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    lexeme[20]='.'; lexeme[21]='.'; lexeme[22]='.'; lexeme[23]='\0';
                    SymbolTableEntry* idLSte = searchToken(symbolTable, lexeme);
                    if(!idLSte){
                        idLSte = createToken(lexeme, ID_LENGTH_EXC, 0);
                        insertToken(symbolTable, idLSte);
                    }
                    tokenInfo* idExTok = createNewNode(idLSte, *lineNumber);
                    ch = nextChar(fp, twinBuff, fwdPtr);
                    for(; ch>='b' && ch<='d'; ch=nextChar(fp, twinBuff, fwdPtr));
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;
                    if(ch>='2' && ch<='7'){
                        state=4; break;
                    }
                    return idExTok;
                }

                break;

            case 81: 
                ch = nextChar(fp, twinBuff, fwdPtr);

                if(!isdigit(ch)){
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;

                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    SymbolTableEntry* cmnSte = searchToken(symbolTable, lexeme);
                    if(cmnSte){
                        tokenInfo* cmnTok = createNewNode(cmnSte, *lineNumber);
                        return cmnTok;
                    }
                    // int l = strlen(lexeme);
                    // if(l>30){
                    //     SymbolTableEntry* errSte = createToken(lexeme, LEXICAL_ERROR, 0);
                    //     insertToken(symbolTable, errSte);
                    //     tokenInfo* errTok = createNewNode(errSte, *lineNumber);
                    //     return errTok;
                    // }
                    SymbolTableEntry* fnidSte = createToken(lexeme, FUNID, 0);
                    insertToken(symbolTable, fnidSte);
                    tokenInfo* fnidTok = createNewNode(fnidSte, *lineNumber);
                    return fnidTok;
                }
                if(getLength(beginPtr, *fwdPtr)>30){
                    tokenizeLexeme(beginPtr, fwdPtr, lexeme, twinBuff);
                    lexeme[30]='.'; lexeme[31]='.'; lexeme[32]='.'; lexeme[33]='\0';
                    SymbolTableEntry* fnLSte = searchToken(symbolTable, lexeme);
                    if(!fnLSte){
                        fnLSte = createToken(lexeme, FUN_LENGTH_EXC, 0);
                        insertToken(symbolTable, fnLSte);
                    }
                    tokenInfo* fnExTok = createNewNode(fnLSte, *lineNumber);
                    ch = nextChar(fp, twinBuff, fwdPtr);
                    for(; isdigit(ch); ch=nextChar(fp, twinBuff, fwdPtr));
                    --(*fwdPtr); if(*fwdPtr<0) *fwdPtr += 2*BUFFER_SIZE;
                    if(*fwdPtr==BUFFER_SIZE-1 || *fwdPtr==2*BUFFER_SIZE-1) specialThing=true;
                    return fnExTok;
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
    // Fills up the Trie (for keywords lookup) with all the keywords.
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

void initializeTokenToString(){
    // Initializes the array tokenToString that'll be used to retrieve the corresponding string for a given token's enum.
    if(tkSInit) return;
    tkSInit=true;
    for(int i=0; i<TK_NOT_FOUND; i++){
        tokenToString[i] = malloc(TOKEN_NAME_LENGTH);
    }
    tokenToString[ASSIGNOP] = "TK_ASSIGNOP";
    tokenToString[COMMENT] = "TK_COMMENT"; 
    tokenToString[FIELDID] = "TK_FIELDID"; 
    tokenToString[ID] = "TK_ID"; 
    tokenToString[NUM] = "TK_NUM"; 
    tokenToString[RNUM] = "TK_RNUM"; 
    tokenToString[FUNID] = "TK_FUNID"; 
    tokenToString[RUID] = "TK_RUID"; 
    tokenToString[WITH] = "TK_WITH"; 
    tokenToString[PARAMETERS] = "TK_PARAMETERS";
    tokenToString[END] = "TK_END"; 
    tokenToString[WHILE] = "TK_WHILE";
    tokenToString[UNION] = "TK_UNION";
    tokenToString[ENDUNION] = "TK_ENDUNION";
    tokenToString[DEFINETYPE] = "TK_DEFINETYPE";
    tokenToString[AS] = "TK_AS";
    tokenToString[TYPE] = "TK_TYPE";
    tokenToString[MAIN] = "TK_MAIN";
    tokenToString[GLOBAL] = "TK_GLOBAL";
    tokenToString[PARAMETER] = "TK_PARAMETER";
    tokenToString[LIST] = "TK_LIST"; 
    tokenToString[SQL] = "TK_SQL";
    tokenToString[SQR] = "TK_SQR";
    tokenToString[INPUT] = "TK_INPUT";
    tokenToString[OUTPUT] = "TK_OUTPUT";
    tokenToString[INT] = "TK_INT";
    tokenToString[REAL] = "TK_REAL";
    tokenToString[COMMA] = "TK_COMMA";
    tokenToString[SEM] = "TK_SEM";
    tokenToString[IF] = "TK_IF";
    tokenToString[COLON] = "TK_COLON";
    tokenToString[DOT] = "TK_DOT";
    tokenToString[ENDWHILE] = "TK_ENDWHILE";
    tokenToString[OP] = "TK_OP";
    tokenToString[CL] = "TK_CL";
    tokenToString[THEN] = "TK_THEN";
    tokenToString[ENDIF] = "TK_ENDIF";
    tokenToString[READ] = "TK_READ";
    tokenToString[WRITE] = "TK_WRITE";
    tokenToString[RETURN] = "TK_RETURN";
    tokenToString[PLUS] = "TK_PLUS";
    tokenToString[MINUS] = "TK_MINUS";
    tokenToString[MUL] = "TK_MUL";
    tokenToString[DIV] = "TK_DIV";
    tokenToString[CALL] = "TK_CALL";
    tokenToString[RECORD] = "TK_RECORD";
    tokenToString[ENDRECORD] = "TK_ENDRECORD";
    tokenToString[ELSE] = "TK_ELSE";
    tokenToString[AND] = "TK_AND";
    tokenToString[OR] = "TK_OR";
    tokenToString[NOT] = "TK_NOT";
    tokenToString[LT] = "TK_LT";
    tokenToString[LE] = "TK_LE";
    tokenToString[EQ] = "TK_EQ";
    tokenToString[GT] = "TK_GT";
    tokenToString[GE] = "TK_GE";
    tokenToString[NE] = "TK_NE";
    tokenToString[EPS] = "TK_EPS";
    tokenToString[DOLLAR] = "TK_DOLLAR";
    tokenToString[LEXICAL_ERROR] = "LEXICAL_ERROR";
    tokenToString[ID_LENGTH_EXC] = "IDENTIFIER_LENGTH_EXCEEDED";
    tokenToString[FUN_LENGTH_EXC] = "FUNCTION_NAME_LENGTH_EXCEEDED";
}

linkedList* getAllTokens(FILE* fp){
    // This function calls the necessary initialization functions and retrieves and returns a list of tokens 
    // tokenized by the lexer with each call to getNextToken()
    char twinBuffer[BUFFER_SIZE*2];
    int fwdPtr = 2*BUFFER_SIZE-1;
    int lineNumber=1;

    Trie* keywordsLookup=createTrie();
    initializeKeywordsLookup(keywordsLookup);
    initializeTokenToString();
    
    SymbolTable* symbolTable = createSymbolTable();

    linkedList* tokensList = createNewList();

    for(; true ; ){
        tokenInfo* tkInfo = getNextToken(fp, twinBuffer, &fwdPtr, &lineNumber, keywordsLookup, symbolTable);
        if(!tkInfo){
            printf("No token retrieved\n");
            break;
        }
        // if(tkInfo->STE->tokenType!=COMMENT)
            insertLLNode(tokensList, tkInfo);
        // printf("itr %d\n", i);

        if(tkInfo->STE->tokenType==DOLLAR) break;
    }
    return tokensList;
}




void removeComments(char* testcaseFile, char* cleanFile) {

    FILE* inputfile = fopen(testcaseFile, "r"); //should be changed to "rb" if our language has a different file extension

    if(inputfile == NULL) {
        printf("Error opening the specified file!\n");
        return;
    }

    char buf[4096]; //buffer to temporarily hold lines
    // FILE* outputfile = fopen(cleanFile, "w");
    char* commentSym;
    
    while(fgets(buf, sizeof(buf), inputfile) != NULL) { //loop through each line

        commentSym = strchr(buf, '%'); //strchr finds the position of % in the line
        if (commentSym != NULL) { //not a null pointer means % found
            *commentSym = '\n'; //change line
            *(commentSym+1) = '\0'; //truncate the rest of the line
        }
        printf("%s",buf);
        // fputs(buf, outputfile); //copy the modified line
    }

    fclose(inputfile);
    // fclose(outputfile);

}

void printTokensOnConsole(linkedList* theListOfTokens){
    // Prints the list on tokens retrieved from lexer on the console
    tokenInfo* tmp = theListOfTokens->head;
    for(int i=0; i<theListOfTokens->count && tmp; i++){
        char* tms;
        if(tmp->STE->tokenType < LEXICAL_ERROR) tms = strdup(tokenToString[tmp->STE->tokenType]);
        else if(tmp->STE->tokenType == LEXICAL_ERROR) tms = strdup("Unrecognized pattern");
        else if(tmp->STE->tokenType == ID_LENGTH_EXC) tms = strdup("Identifier length exceeded 20");
        else if(tmp->STE->tokenType == FUN_LENGTH_EXC) tms = strdup("Function name length exceeded 30");
        else tms = strdup("");
        printf("Line No: %*d \t Lexeme: %*s \t Token: %*s \n", 5, tmp->lineNumber, 35, tmp->STE->lexeme, 35, tms);
        tmp = tmp->next;
    }
}

linkedList* LexInput(FILE* fp, char* outp){
    // Wrapper function that calls the getAllTokens to get and return all the tokens tokenized by lexer
    // This function will be called by the driver

    if(!fp){
        printf("Error opening input file for lexer\n");
        exit(-1);
    }
    

    linkedList* theList = getAllTokens(fp);
    if(!theList){
        printf("Could not retrieve list of tokens\n");
        exit(-1);
    }

    tokenInfo* tmp = theList->head;
    
    
    return theList;
}