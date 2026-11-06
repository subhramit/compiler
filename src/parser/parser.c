/*
#                    GROUP - 8
# 2020B1A70630P                       Aditya Thakur
# 2021A7PS2001P                       Amal Sayeed
# 2021A7PS2005P                       Ohiduz Zaman
# 2021A7PS2682P                       Priyansh Patel
# 2021A7PS2002P                       Rachoita Das
# 2020B1A70611P                       Subhramit Basu Bhowmick
*/

#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include "../lexer/lexer.h"
#include "../lexer/lexerDef.h"
#include "parser.h"
#include "parserDef.h"
#include "stack.h"


char* nonTerminalToString[NT_NOT_FOUND];
grammarRule* Grammar[MAX_GRAMMAR_RULES];
int  numOfRules=0;
ffRhs** First;
ffRhs** Follow;
ffRhs** AutoFirst;
ffRhs** AutoFollow;
grammarRule*** parseTable;

bool grmRead=false;
bool ntInit=false;
bool ffInit=false;
bool ptInit=false;

//Stack ADT
Stack* createNewStack() {
    // Creates a new stack (linked list implementation of stack)
    Stack* newStack = (Stack*)malloc(sizeof(Stack));

    if (newStack == NULL) {
        printf("Memory allocation failure for new stack\n");
        exit(-1);
    }

    newStack->top = NULL;
    return newStack;
}

void push(Stack* stack, pTreeNode* ptn) {
    // Pushes an element onto the top of the stack.
    stackNODE newNode = (stackNODE)malloc(sizeof(stackNode));
    
    if (newNode == NULL) {
        printf("Memory allocation failure while creating new node for pushing\n");
        exit(-1);
    }

    newNode->data = ptn;
    newNode->next = stack->top;
    stack->top = newNode;
}

void pop(Stack* stack) {
    // Pops the top element of the stack if not empty
    if (!isEmpty(stack)) {
        stackNODE temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
    }
}

pTreeNode* top(Stack* stack) {
    // returns the top element of the stack if not empty, and null otherwise
    if(!isEmpty(stack)) 
        return stack->top->data;
    else 
        return NULL;
}

bool isEmpty(Stack* stack) { 
    return (stack->top == NULL);
}


symbolListNode* createSymbolListNode(grammarSymbol* gs){
    // structure for a node in the linked list of grammar rules

    symbolListNode* newNode = (symbolListNode*)malloc(sizeof(symbolListNode));
    if(!newNode){
        printf("Could not allocate memory for symbol List Node\n"); return NULL;
    }
    newNode->next = NULL; newNode->prev = NULL;
    newNode->symb = gs;
    return newNode;
}

symbolList* createSymbolList(){
    // creates the linked list for a grammar rule's RHS

    symbolList* newList = (symbolList*) malloc(sizeof(symbolList));
    if(!newList){
        printf("Could not allocate memory for Symbol List\n"); return NULL;
    }
    newList->count=0;
    newList->head=NULL; newList->tail=NULL;
    return newList;
}

void insertSymbolListNode(symbolList* symList, symbolListNode* symListNode){ 
    // Inserts a node into the linked list

    if(symList->tail==NULL)
        symList->head = symListNode;
    else{
        symList->tail->next = symListNode;
        symListNode->prev = symList->tail;
    }
    symList->tail = symListNode;
    ++(symList->count);
}

pTreeNode* createPTreeNode(){
    // Creates a node for the parse tree

    pTreeNode* newTreeNode = (pTreeNode*) malloc(sizeof(pTreeNode));
    if(!newTreeNode){
        printf("Couldnt allocate memory for creating parse tree node\n"); return NULL;
    }
    newTreeNode->children = (pTreeNode**) malloc(INITIAL_CHILDREN_CAPACITY*sizeof(pTreeNode*));
    if(!(newTreeNode->children)){
        printf("Couldnt allocate memory for parse tree node's children\n");
        free(newTreeNode);
        return NULL;
    }
    newTreeNode->capacity = INITIAL_CHILDREN_CAPACITY;
    newTreeNode->size = 0;
    newTreeNode->symbol = NULL;
    newTreeNode->ste = NULL;
    newTreeNode->lineNumber = -1;
    for(int i=0; i<INITIAL_CHILDREN_CAPACITY; i++) newTreeNode->children[i] = NULL;
    
    return newTreeNode;
}

pTree* createPTree(){
    // Creates the parse tree structure

    pTree* newTree = (pTree*) malloc(sizeof(pTree));
    if(!newTree){
        printf("Could not allocate memory for creating parse tree\n"); return NULL;
    }
    newTree->root = createPTreeNode();
    if(!newTree->root){ free(newTree); return NULL; }
    return newTree;
}

void insertNodeAsChild(pTreeNode* par, pTreeNode* child){
    // Inserts the node child as a child of the node par

    if(!par){
        printf("Parent node is null. Couldnt add child\n"); return;
    }
    if(!child){
        printf("Child node is null. Couldnt add child\n"); return;
    }
    if(par->size == par->capacity){ // If the number of children reaches parent's capacity, double parent's capacity
        par->capacity *= 2;
        par->children = (pTreeNode**) realloc(par->children, par->capacity*sizeof(pTreeNode*)); 
        if(!(par->children)){
            printf("Could not allocate memory for resizing node's children capacity\n"); return;
        }
    }
    par->children[(par->size)++] = child;
}


void initializeNonTerminalToString(){
    /*
        Initializes the array nonTerminalToString which will be used to fetch the string corresponding to a 
        given non terminal's enum
    */
    if(ntInit) return; ntInit=true;

    nonTerminalToString[program] = "<program>";
    nonTerminalToString[otherFunctions] = "<otherFunctions>";
    nonTerminalToString[mainFunction] = "<mainFunction>";
    nonTerminalToString[stmts] = "<stmts>";
    nonTerminalToString[stmt] = "<stmt>";
    nonTerminalToString[function] = "<function>";
    nonTerminalToString[input_par] = "<input_par>";
    nonTerminalToString[output_par] = "<output_par>";
    nonTerminalToString[parameter_list] = "<parameter_list>";
    nonTerminalToString[dataType] = "<dataType>";
    nonTerminalToString[remaining_list] = "<remaining_list>";
    nonTerminalToString[primitiveDatatype] = "<primitiveDatatype>";
    nonTerminalToString[constructedDatatype] = "<constructedDatatype>";
    nonTerminalToString[typeDefinitions] = "<typeDefinitions>"; 
    nonTerminalToString[actualOrRedefined] = "<actualOrRedefined>"; 
    nonTerminalToString[typeDefinition] = "<typeDefinition>"; 
    nonTerminalToString[declarations] = "<declarations>"; 
    nonTerminalToString[declaration] = "<declaration>"; 
    nonTerminalToString[otherStmts] = "<otherStmts>"; 
    nonTerminalToString[returnStmt] = "<returnStmt>"; 
    nonTerminalToString[definetypestmt] = "<definetypestmt>";
    nonTerminalToString[fieldDefinition] = "<fieldDefinition>";
    nonTerminalToString[fieldType] = "<fieldType>"; 
    nonTerminalToString[fieldDefinitions] = "<fieldDefinitions>";
    nonTerminalToString[moreFields] = "<moreFields>"; 
    nonTerminalToString[global_or_not] = "<global_or_not>"; 
    nonTerminalToString[assignmentStmt] = "<assignmentStmt>"; 
    nonTerminalToString[iterativeStmt] = "<iterativeStmt>"; 
    nonTerminalToString[conditionalStmt] = "<conditionalStmt>"; 
    nonTerminalToString[elsePart] = "<elsePart>"; 
    nonTerminalToString[ioStmt] = "<ioStmt>"; 
    nonTerminalToString[funCallStmt] = "<funCallStmt>";
    nonTerminalToString[SingleOrRecId] = "<singleOrRecId>";
    nonTerminalToString[option_single_constructed] = "<option_single_constructed>"; 
    nonTerminalToString[oneExpansion] = "<oneExpansion>"; 
    nonTerminalToString[moreExpansions] = "<moreExpansions>"; 
    nonTerminalToString[arithmeticExpression] = "<arithmeticExpression>";
    nonTerminalToString[expPrime] = "<expPrime>"; 
    nonTerminalToString[term] = "<term>"; 
    nonTerminalToString[termPrime] = "<termPrime>";
    nonTerminalToString[factor] = "<factor>"; 
    nonTerminalToString[highPrecedenceOperators] = "<highPrecedenceOperators>";
    nonTerminalToString[lowPrecedenceOperators] = "<lowPrecedenceOperators>"; 
    nonTerminalToString[outputParameters] = "<outputParameters>"; 
    nonTerminalToString[inputParameters] = "<inputParameters>"; 
    nonTerminalToString[idList] = "<idList>"; 
    nonTerminalToString[booleanExpression] = "<booleanExpression>";
    nonTerminalToString[var] = "<var>";  
    nonTerminalToString[logicalOp] = "<logicalOp>";
    nonTerminalToString[relationalOp] = "<relationalOp>"; 
    nonTerminalToString[optionalReturn] = "<optionalReturn>"; 
    nonTerminalToString[more_ids] = "<more_ids>"; 
    nonTerminalToString[A] = "<A>"; 
}

Token getTokenFromString(char* str){
    // Given a string, returns the corresponding token's enum
    
    for(int i=0; i<TK_NOT_FOUND; i++){
        if(!strcmp(tokenToString[i], str))  return (Token)i;
    }
    return TK_NOT_FOUND;
}

NonTerminal getNonTerminalFromString(char* str){
    // Given a string, returns the corresponding non-terminal's enum
    for(int i=0; i<NT_NOT_FOUND; i++){
        if(!strcmp(nonTerminalToString[i], str)) return  (NonTerminal)i;
    }
    return NT_NOT_FOUND;
}

void readGrammar(){
    // Reads the grammar from the input file grammar.txt and stores in an array of structs

    if(grmRead) return;
    /* A failed grammar read must not leave rules to append on a retry. */
    for(int i=0; i<numOfRules; ++i){
        symbolListNode* node = Grammar[i]->rhs->head;
        while(node){
            symbolListNode* next = node->next;
            free(node->symb);
            free(node);
            node = next;
        }
        free(Grammar[i]->rhs);
        free(Grammar[i]->lhs);
        free(Grammar[i]);
        Grammar[i] = NULL;
    }
    numOfRules = 0;
    char buff[MAX_RULE_LENGTH];
    FILE* fp = fopen("src/parser/grammar.txt", "r");
#ifdef COMPILER_ROOT
    if(!fp) fp = fopen(COMPILER_ROOT "/src/parser/grammar.txt", "r");
#endif
    if(!fp){
        fprintf(stderr, "Could not open src/parser/grammar.txt (cwd or COMPILER_ROOT)\n");
        return;
    }
    char* oneTok;
    for(; fgets(buff, MAX_RULE_LENGTH, fp); ){
        // printf("io\n"); fflush(stdout);
        oneTok = strtok(buff, " \t\n\r");
        if(!oneTok) continue;
        if(numOfRules >= MAX_GRAMMAR_RULES ||
           getNonTerminalFromString(oneTok) == NT_NOT_FOUND){
            fprintf(stderr, "Invalid grammar rule or too many grammar rules\n");
            fclose(fp);
            return;
        }

        grammarRule* gRule = (grammarRule*) malloc(sizeof(grammarRule));
        if(!gRule){
            printf("Could not allocate memory for rule\n"); return;
        }
        gRule->lhs = (grammarSymbol*) malloc(sizeof(grammarSymbol));
        if(!(gRule->lhs)){
            printf("Could not allocate memory for grammar symbol\n"); return;
        }
        gRule->lhs->isNonTerminal = true;
        gRule->lhs->tOrNt.nt = getNonTerminalFromString(oneTok);
        gRule->rhs = createSymbolList();

        oneTok = strtok(NULL, " \t\n\r");

        for(; oneTok; ){
            // printf("guho\n"); fflush(stdout);
            grammarSymbol* gsym = (grammarSymbol*) malloc(sizeof(grammarSymbol));
            if(!gsym){
                printf("Could not allocate memory for rhs grammar symbol\n"); break;
            }
            if(oneTok[0]=='<'){
                gsym->isNonTerminal = true;
                gsym->tOrNt.nt = getNonTerminalFromString(oneTok);
            }
            else{
                char tmp[TOKEN_NAME_LENGTH] = "TK_";
                if(strlen(oneTok) + 4 > sizeof(tmp)){
                    fprintf(stderr, "Grammar token name is too long\n");
                    fclose(fp);
                    return;
                }
                strcat(tmp, oneTok);
                gsym->isNonTerminal = false;
                gsym->tOrNt.t = getTokenFromString(tmp);
            }
            if((gsym->isNonTerminal && gsym->tOrNt.nt == NT_NOT_FOUND) ||
               (!gsym->isNonTerminal && gsym->tOrNt.t == TK_NOT_FOUND)){
                fprintf(stderr, "Unknown grammar symbol: %s\n", oneTok);
                free(gsym);
                fclose(fp);
                return;
            }
            symbolListNode* rhsNode = createSymbolListNode(gsym);
            insertSymbolListNode(gRule->rhs, rhsNode);

            oneTok = strtok(NULL, " \t\n\r");
        }
        if(!gRule->rhs->head){
            fprintf(stderr, "Grammar rule has an empty RHS\n");
            fclose(fp);
            return;
        }
        Grammar[numOfRules++] = gRule;
    }
    grmRead = !ferror(fp) && numOfRules > 0;
    fclose(fp);
}

void printGrammar(){
    FILE* fp = fopen("readgrm.txt", "w");
    for(int i=0; i<numOfRules; i++){
        grammarRule* tmpRule = Grammar[i];
        fprintf(fp, "%d. %s: ", i+1, nonTerminalToString[tmpRule->lhs->tOrNt.nt]);

        symbolListNode* tsNode = tmpRule->rhs->head;
        for(int j=0; j<tmpRule->rhs->count; j++){
            if(tsNode->symb->isNonTerminal)
                fprintf(fp, "%s\t", nonTerminalToString[tsNode->symb->tOrNt.nt]);
            else    
                fprintf(fp, "%s\t", tokenToString[tsNode->symb->tOrNt.t]);
            tsNode=tsNode->next;
        }
        fprintf(fp, "\n");
    }
}

void insertFfRhsNode(ffRhs* lst, ffRhsNode* nd){
    // inserts a node into a list (list of tokens in first set or follow set)

    if(!(lst->tail)){
        lst->head = nd;
        lst->tail = nd;
    }
    else{
        lst->tail->next = nd;
        lst->tail = nd;
    }
}


void printComputedFirstAndFollow(){
    FILE* foutFirst = fopen("computedfrst.txt", "w");
    for(int i=0; i<NT_NOT_FOUND; i++){
        fprintf(foutFirst, "%s:\t", nonTerminalToString[i]);
        ffRhsNode* tmp = AutoFirst[i]->head;
        for(; tmp; tmp=tmp->next){
            fprintf(foutFirst, "%s\t", tokenToString[tmp->tk]);
        }
        fprintf(foutFirst, "\n");
    }
    fclose(foutFirst);

    FILE* foutFollow = fopen("computedfllw.txt", "w");
    for(int i=0; i<NT_NOT_FOUND; i++){
        fprintf(foutFollow, "%s:\t", nonTerminalToString[i]);
        ffRhsNode* tmp = AutoFollow[i]->head;
        for(; tmp; tmp=tmp->next){
            fprintf(foutFollow, "%s\t", tokenToString[tmp->tk]);
        }
        fprintf(foutFollow, "\n");
    }
    fclose(foutFollow);
}

bool existsInRhs(ffRhs* lst, Token tmpkey){
    // checks if a token exists in the given list
    if(!lst) return false;
    ffRhsNode* current = lst->head;
    for(; current; current=current->next){
        if(current->tk==tmpkey) return true;
    }
    return false;
}

ffRhs* getFirstOfRhs(symbolList* grhs){
    // Returns a list containing the "First" of a given grammar's RHS

    ffRhs* res = (ffRhs*) malloc(sizeof(ffRhs));
    res->head = NULL;
    res->tail = NULL;

    symbolListNode* ritr = grhs->head;
    bool hasEps=true;
    for(; ritr && hasEps; ritr=ritr->next){
        ffRhsNode* tmp;  
        hasEps=false;  
        if(!(ritr->symb->isNonTerminal)){
            if(!existsInRhs(res, ritr->symb->tOrNt.t)){
                ffRhsNode* vtmp = (ffRhsNode*) malloc(sizeof(ffRhsNode));
                vtmp->next=NULL; vtmp->tk = ritr->symb->tOrNt.t;
                insertFfRhsNode(res, vtmp);
            }
            break;
        }
        for(tmp=AutoFirst[ritr->symb->tOrNt.nt]->head; tmp; tmp=tmp->next){
            if(tmp->tk==EPS){
                hasEps=true; continue;
            }
            if(!existsInRhs(res, tmp->tk)){
                ffRhsNode* cpytmp = (ffRhsNode*) malloc(sizeof(ffRhsNode));
                cpytmp->next=NULL;
                cpytmp->tk = tmp->tk;
                insertFfRhsNode(res, cpytmp);
            }
        }

    }
    if(ritr==NULL && hasEps){
        ffRhsNode* vvtmp = (ffRhsNode*) malloc(sizeof(ffRhsNode));
        vvtmp->next=NULL; vvtmp->tk = EPS;
        if(!existsInRhs(res, EPS)) 
            insertFfRhsNode(res, vvtmp);
    }
    return res;
}

void freeUpFfRhs(ffRhs* rl){
    // Frees up memory for the RHS list of a grammar or First/Follow set

    if(!rl) return;
    ffRhsNode *tmp, *tn;
    tmp = rl->head;
    for(; tmp; ){
        tn = tmp;
        tmp=tmp->next;
        free(tn);
    }
    free(rl);
}

void addRulesToParseTable(){
    //  Adds all rules to the parse table 

    for(int gri=0; gri<numOfRules; ++gri){
        grammarRule* tmpRule = Grammar[gri];
        ffRhs* fOfRhs = getFirstOfRhs(tmpRule->rhs);
        ffRhsNode* tItr;
        for(tItr = fOfRhs->head; tItr; tItr=tItr->next){
            if(parseTable[tmpRule->lhs->tOrNt.nt][tItr->tk]!=NULL)
                printf("\nMultiple defined entries in parse table detected! (Overwriting the rule!)\n");
            parseTable[tmpRule->lhs->tOrNt.nt][tItr->tk]=tmpRule;
        }

        if(existsInRhs(fOfRhs, EPS)){
            ffRhsNode* antItr;
            for(antItr=AutoFollow[tmpRule->lhs->tOrNt.nt]->head; antItr; antItr=antItr->next){
                if(parseTable[tmpRule->lhs->tOrNt.nt][antItr->tk]!=NULL)
                    printf("\nMultiple defined entries in parse table detected! (Overwriting the rule!)\n");
                parseTable[tmpRule->lhs->tOrNt.nt][antItr->tk]=tmpRule;
            }
        }

        freeUpFfRhs(fOfRhs);
    }
}

void initializeParseTable(){
    // Creates the parse table structure and calls the initialization of parse table

    if(ptInit) return; ptInit=true;
    parseTable = (grammarRule***) malloc(NT_NOT_FOUND*sizeof(grammarRule**));
    if(!parseTable){
        printf("Couldn't allocate memory for parseTable rows\n");
        return;
    }
    for(int nti=0; nti<NT_NOT_FOUND; nti++){
        parseTable[nti] = (grammarRule**) malloc(TK_NOT_FOUND*sizeof(grammarRule*));
        if(!parseTable[nti]){
            printf("Couldn't allocate memory for parseTable columns\n");
            return;
        }
        for(int tki=0; tki<TK_NOT_FOUND; tki++){
            parseTable[nti][tki] = NULL;
        }
    }

    addRulesToParseTable();
}

void printRule(grammarRule* rule, FILE* fp){
    fprintf(fp, "Lhs: %s\n", nonTerminalToString[rule->lhs->tOrNt.nt]);
    fprintf(fp, "Rhs: ");
    symbolListNode* tmp;
    for(tmp = rule->rhs->head; tmp; tmp=tmp->next){
        fprintf(fp, "%s\t", tmp->symb->isNonTerminal ? nonTerminalToString[tmp->symb->tOrNt.nt] : tokenToString[tmp->symb->tOrNt.t]);
    }
    fprintf(fp, "\n");
}

void printParseTable(){
    FILE* fParseOut = fopen("parseTable.txt", "w");
    if(!fParseOut){
        printf("Could not open file for printing parse table\n"); return;
    }
    for(int i=0; i<NT_NOT_FOUND; i++){
        fprintf(fParseOut, "------------------------------Non Term %d -----------------------------\n", i);
        for(int j=0; j<TK_NOT_FOUND; j++){
            fprintf(fParseOut, "******Term %d ******\n", j);
            grammarRule* tmpRule = parseTable[i][j];
            if(tmpRule)
                printRule(tmpRule, fParseOut);
            else
                fprintf(fParseOut, "Error Entry\n");
        }
    }
    fclose(fParseOut);
}

static void freeTreeNode(pTreeNode* node){
    if(!node) return;
    for(int i=0; i<node->size; ++i) freeTreeNode(node->children[i]);
    free(node->children);
    free(node->symbol);
    free(node);
}

void freeParseTree(pTree* tree){
    if(!tree) return;
    freeTreeNode(tree->root);
    free(tree);
}

static void parseError(int* errors, bool report, int line, const char* message,
                       const char* lexeme, const char* expected){
    ++(*errors);
    if(report) fprintf(stderr, "Line %d: %s \"%s\" (expected %s)\n",
                       line, message, lexeme, expected);
}

static pTree* parseTokens(linkedList* tokensFromLexer, int* errors, bool report){
    tokenInfo* inputPtr = tokensFromLexer ? tokensFromLexer->head : NULL;
    pTree* tree = createPTree();
    if(!tree){
        parseError(errors, report, 0, "Cannot allocate parse tree", "", "memory");
        return NULL;
    }
    tree->root->symbol = malloc(sizeof(grammarSymbol));
    if(!tree->root->symbol){
        freeParseTree(tree);
        parseError(errors, report, 0, "Cannot allocate root symbol", "", "memory");
        return NULL;
    }
    tree->root->symbol->isNonTerminal = true;
    tree->root->symbol->tOrNt.nt = program;
    Stack* stack = createNewStack();
    push(stack, tree->root);
    int line = 1;
    bool sawEOF = false;
    static SymbolTableEntry epsilon = {"EPSILON", EPS, 0};

    while(inputPtr){
        Token token = inputPtr->STE->tokenType;
        line = inputPtr->lineNumber;
        if(token == COMMENT){ inputPtr = inputPtr->next; continue; }
        if(token >= LEXICAL_ERROR){
            parseError(errors, report, line, "Lexical error", inputPtr->STE->lexeme,
                       token == ID_LENGTH_EXC ? "identifier length <= 20" :
                       token == FUN_LENGTH_EXC ? "function name length <= 30" : "valid token");
            inputPtr = inputPtr->next;
            continue;
        }
        if(sawEOF){
            parseError(errors, report, line, "Token after EOF", inputPtr->STE->lexeme, "end of input");
            inputPtr = inputPtr->next;
            continue;
        }
        if(isEmpty(stack)){
            if(token == DOLLAR) sawEOF = true;
            else parseError(errors, report, line, "Trailing token", inputPtr->STE->lexeme, "EOF");
            inputPtr = inputPtr->next;
            continue;
        }
        pTreeNode* current = top(stack);
        grammarSymbol* symbol = current->symbol;
        current->lineNumber = line;
        if(!symbol->isNonTerminal && symbol->tOrNt.t == EPS){
            current->ste = &epsilon;
            pop(stack);
        }
        else if(!symbol->isNonTerminal){
            if(symbol->tOrNt.t == token){
                current->ste = inputPtr->STE;
                inputPtr = inputPtr->next;
            }
            else parseError(errors, report, line, "Unexpected token", inputPtr->STE->lexeme,
                            tokenToString[symbol->tOrNt.t]);
            pop(stack);
        }
        else{
            grammarRule* rule = parseTable[symbol->tOrNt.nt][token];
            if(!rule){
                parseError(errors, report, line, "Invalid token", inputPtr->STE->lexeme,
                           nonTerminalToString[symbol->tOrNt.nt]);
                if(token == DOLLAR || existsInRhs(AutoFollow[symbol->tOrNt.nt], token)) pop(stack);
                else inputPtr = inputPtr->next;
                continue;
            }
            pop(stack);
            for(symbolListNode* rhs = rule->rhs->head; rhs; rhs = rhs->next){
                pTreeNode* child = createPTreeNode();
                if(!child) goto allocationFailure;
                child->symbol = malloc(sizeof(grammarSymbol));
                if(!child->symbol){ freeTreeNode(child); goto allocationFailure; }
                *child->symbol = *rhs->symb;
                insertNodeAsChild(current, child);
            }
            for(int i=current->size-1; i>=0; --i) push(stack, current->children[i]);
        }
    }
    if(!sawEOF) parseError(errors, report, line, "Missing EOF marker", "", "EOF");
    while(!isEmpty(stack)){
        pTreeNode* current = top(stack);
        parseError(errors, report, line, "Unexpected end of input", "",
                   current->symbol->isNonTerminal ? nonTerminalToString[current->symbol->tOrNt.nt] :
                   tokenToString[current->symbol->tOrNt.t]);
        pop(stack);
    }
    free(stack);
    return tree;

allocationFailure:
    parseError(errors, report, line, "Cannot allocate parse tree node", "", "memory");
    while(!isEmpty(stack)) pop(stack);
    free(stack);
    freeParseTree(tree);
    return NULL;
}

void computeFirstSets(){
    //Computes the first sets for the non terminals in the grammar
    bool modified = true;
    for(; modified; ){
        modified = false;

        for(int gri=0; gri<numOfRules; ++gri){
            NonTerminal currLhs = Grammar[gri]->lhs->tOrNt.nt;
            symbolListNode* rhsItr = Grammar[gri]->rhs->head;

            bool hasEps = true;
            for(; rhsItr && hasEps; rhsItr=rhsItr->next){
                hasEps = false;
                
                if(!(rhsItr->symb->isNonTerminal)){
                    if(!existsInRhs(AutoFirst[currLhs], rhsItr->symb->tOrNt.t)){
                        ffRhsNode* vtmp = (ffRhsNode*) malloc(sizeof(ffRhsNode));
                        vtmp->next=NULL; vtmp->tk = rhsItr->symb->tOrNt.t;
                        insertFfRhsNode(AutoFirst[currLhs], vtmp);
                        modified = true;
                    }
                    break;
                }

                ffRhsNode* feaItr;
                for(feaItr=AutoFirst[rhsItr->symb->tOrNt.nt]->head; feaItr; feaItr=feaItr->next){
                    if(feaItr->tk==EPS){
                        hasEps=true; continue;
                    }
                    if(!existsInRhs(AutoFirst[currLhs], feaItr->tk)){
                        ffRhsNode* cpytmp = (ffRhsNode*) malloc(sizeof(ffRhsNode));
                        cpytmp->next=NULL;
                        cpytmp->tk = feaItr->tk;
                        insertFfRhsNode(AutoFirst[currLhs], cpytmp);
                        modified = true;
                    }
                }

            }
            if(rhsItr==NULL && hasEps){
                if(!existsInRhs(AutoFirst[currLhs], EPS)){
                    ffRhsNode* vvtmp = (ffRhsNode*) malloc(sizeof(ffRhsNode));
                    vvtmp->next=NULL; vvtmp->tk = EPS;
                    insertFfRhsNode(AutoFirst[currLhs], vvtmp);
                    modified = true;
                } 
            }
        }
    }
}

void computeFollowSets(){
    // Computes the follow sets for the non terminals in the grammar

    ffRhsNode* thdl = (ffRhsNode*) malloc(sizeof(ffRhsNode));
    thdl->next = NULL; thdl->tk = DOLLAR;
    insertFfRhsNode(AutoFollow[program], thdl);

    bool modified = true;
    for(; modified; ){
        modified = false;

        for(int gri=0; gri<numOfRules; ++gri){
            NonTerminal currLhs = Grammar[gri]->lhs->tOrNt.nt;
            symbolListNode* rhsItr = Grammar[gri]->rhs->head;

            for(; rhsItr;  rhsItr=rhsItr->next){
                if(!(rhsItr->symb->isNonTerminal)) continue;

                symbolList* tgl = (symbolList*) malloc(sizeof(symbolList));
                tgl->head=rhsItr->next;
                ffRhs* fOfNxtT = getFirstOfRhs(tgl);

                ffRhsNode* felItr;
                for(felItr=fOfNxtT->head; felItr; felItr=felItr->next){
                    if(felItr->tk==EPS) continue;
                    if(!existsInRhs(AutoFollow[rhsItr->symb->tOrNt.nt], felItr->tk)){
                        ffRhsNode* vvvt = (ffRhsNode*) malloc(sizeof(ffRhs));
                        vvvt->next=NULL; vvvt->tk = felItr->tk;
                        insertFfRhsNode(AutoFollow[rhsItr->symb->tOrNt.nt], vvvt);
                        modified = true;
                    }
                }

                if(existsInRhs(fOfNxtT, EPS) || !(rhsItr->next)){
                    ffRhsNode* flwItr;
                    for(flwItr=AutoFollow[currLhs]->head; flwItr; flwItr=flwItr->next){
                        if(!existsInRhs(AutoFollow[rhsItr->symb->tOrNt.nt], flwItr->tk)){
                            ffRhsNode* avt = (ffRhsNode*) malloc(sizeof(ffRhs));
                            avt->next = NULL; avt->tk = flwItr->tk;
                            insertFfRhsNode(AutoFollow[rhsItr->symb->tOrNt.nt], avt);
                            modified = true;
                        }
                    }
                }
                freeUpFfRhs(fOfNxtT);
                free(tgl);
            }
        }
    }
}

void initializeAndComputeFirstAndFollow(){
    // Creates the data structures for the first and follow and calls computeFirstSets() and computeFollowSets()

    if(ffInit) return; ffInit=true;
    AutoFirst = (ffRhs**) malloc(NT_NOT_FOUND*sizeof(ffRhs*));
    AutoFollow = (ffRhs**) malloc(NT_NOT_FOUND*sizeof(ffRhs*));
    if(!AutoFirst || !AutoFollow){
        printf("Could not allocate memory for auto first and follow sets array\n");
        return;
    }

    for(int i=0; i<NT_NOT_FOUND; i++){
        AutoFirst[i] = (ffRhs*) malloc(sizeof(ffRhs));
        if(!(AutoFirst[i])){
            printf("Could not allocate memory for auto first sets\n");
            return;
        }
        AutoFirst[i]->head = NULL; AutoFirst[i]->tail = NULL;
        AutoFollow[i] = (ffRhs*) malloc(sizeof(ffRhs));
        if(!(AutoFollow[i])){
            printf("Could not allocate memory for auto follow sets\n");
            return;
        }
        AutoFollow[i]->head = NULL; AutoFollow[i]->tail = NULL;
    }
    computeFirstSets();
    computeFollowSets();
}

void printTreeNode(pTreeNode* curr, pTreeNode* par, FILE* fp){
    // Prints the details of a specific tree node on a line in the specified file following the format provided

    // fprintf(fp, "\n");
    fprintf(fp, "%*s ", 32, !(curr->symbol->isNonTerminal) ? curr->ste->lexeme : "-----");
    fprintf(fp, "%*d ", 12, curr->lineNumber);
    fprintf(fp, "%*s ", 16, curr->symbol->isNonTerminal ? "-----" : tokenToString[curr->ste->tokenType]);
    if(!(curr->symbol->isNonTerminal) && (curr->ste->tokenType==NUM || curr->ste->tokenType==RNUM)){
        if(curr->ste->tokenType==NUM)
            fprintf(fp, "%20.0f ", curr->ste->valueIfNumber);
        else    
            fprintf(fp, "%20.2lf ", curr->ste->valueIfNumber);
    }
    else{
        fprintf(fp, "%*s ", 20, "Not number ");
    }
    fprintf(fp, "%*s ", 30, par ? nonTerminalToString[par->symbol->tOrNt.nt] : "ROOT");
    fprintf(fp, "%*s ", 12, curr->symbol->isNonTerminal ? "NO" : "YES");
    fprintf(fp, "%*s ", 30, curr->symbol->isNonTerminal ? nonTerminalToString[curr->symbol->tOrNt.nt] : "-----");
    fprintf(fp, "\n");
}

void inorderTraverse(pTreeNode* curr, pTreeNode* par, FILE* fp){
    // Traverses the tree inorder and prints node details
    if(!curr) return;

    if(curr->size) inorderTraverse(curr->children[0], curr, fp);
    printTreeNode(curr, par, fp);
    for(int chi=1; chi < curr->size; ++chi) inorderTraverse(curr->children[chi], curr, fp);
}

void printParseTree(pTree* PT, char* outFile){
    FILE* fp = outFile ? fopen(outFile, "w") : NULL;
    if(!fp){
        printf("Could not open file for printing parse tree\n");
        return;
    }
    if(!PT){
        printf("Given parse tree is null. Could not print\n");
        fclose(fp);
        return;
    }
    if(shouldPrint)
        printf("Printing Parse Tree in the specified file...\n");
    fprintf(fp, "%*s %*s %*s %*s %*s %*s %*s\n\n", 32, "lexeme", 12, "lineNumber", 16, "tokenName", 20, "valueIfNumber", 30, "parentNodeSymbol", 12, "isLeafNode", 30, "nodeSymbol");
    inorderTraverse(PT->root, NULL, fp);
    fclose(fp);
    if(shouldPrint)
        printf("Printing parse tree completed...\n");
}

static pTree* parseSourceFileImpl(const char* input, int* errors, bool report){
    int count = 0;
    FILE* fp = input ? fopen(input, "rb") : NULL;
    if(!fp){
        parseError(&count, report, 0, "Cannot open input file", input ? input : "(null)", "readable file");
        if(errors) *errors += count;
        return NULL;
    }
    linkedList* tokens = getAllTokens(fp);
    if(ferror(fp)) parseError(&count, report, 0, "Cannot read input file", input, "readable file");
    fclose(fp);

    initializeTokenToString();
    initializeNonTerminalToString();
    readGrammar();
    pTree* tree = NULL;
    if(!grmRead) parseError(&count, report, 0, "Cannot load grammar", "", "grammar file");
    else if(!tokens) parseError(&count, report, 0, "Lexer failed", "", "token list");
    else{
        initializeAndComputeFirstAndFollow();
        initializeParseTable();
        tree = parseTokens(tokens, &count, report);
    }
    if(tokens){
        tokenInfo* node = tokens->head;
        while(node){
            tokenInfo* next = node->next;
            free(node);
            node = next;
        }
        free(tokens);
    }
    if(count){ freeParseTree(tree); tree = NULL; }
    if(errors) *errors += count;
    return tree;
}

pTree* parseSourceFile(const char* input, int* errors){
    return parseSourceFileImpl(input, errors, true);
}

void parseInputSourceCode(char* inpFile, char* opFile){
    int errors = 0;
    pTree* tree = parseSourceFileImpl(inpFile, &errors, shouldPrint);
    if(tree){
        if(shouldPrint) printf("\nParsing successful! No syntax errors! The input is syntactically correct!\n");
        printParseTree(tree, opFile);
        freeParseTree(tree);
    }
    else{
        FILE* fp = opFile ? fopen(opFile, "w") : NULL;
        if(!fp){
            fprintf(stderr, "Could not open file for printing parse tree\n");
            return;
        }
        fprintf(fp, "There were syntax errors in the input file. Not printing the parse tree!\nCheck the console for error details.");
        fclose(fp);
    }
}