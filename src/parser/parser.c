/*
#                    GROUP - 8
# 2020B1A70630P                       Aditya Thakur
# 2021A7PS2001P                       Amal Sayeed
# 2020A7PS2005P                       Ohiduz Zaman
# 2020A7PS2682P                       Priyansh Patel
# 2020A7PS2002P                       Rachoita Das
# 2020B1A70611P                       Subhramit Basu Bhowmick
*/

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "../lexer/lexer.h"
#include "../lexer/lexerDef.h"
#include "parser.h"
#include "parserDef.h"
#include "stack.h"


//Stack ADT
Stack* createNewStack() {

    Stack* newStack = (Stack*)malloc(sizeof(Stack));

    if (newStack == NULL) {
        printf("Memory allocation failure for new stack\n");
        exit(-1);
    }

    newStack->top = NULL;
    return newStack;
}

void push(Stack* stack, pTreeNode* ptn) {

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

    if (!isEmpty(stack)) {
        stackNODE temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
    }
}

pTreeNode* top(Stack* stack) {

    if(!isEmpty(stack)) 
        return stack->top->data;
    else 
        return NULL;
}

bool isEmpty(Stack* stack) {
    return (stack->top == NULL);
}


symbolListNode* createSymbolListNode(grammarSymbol* gs){
    symbolListNode* newNode = (symbolListNode*)malloc(sizeof(symbolListNode));
    if(!newNode){
        printf("Could not allocate memory for symbol List Node\n"); return NULL;
    }
    newNode->next = NULL; newNode->prev = NULL;
    newNode->symb = gs;
    return newNode;
}

symbolList* createSymbolList(){
    symbolList* newList = (symbolList*) malloc(sizeof(symbolList));
    if(!newList){
        printf("Could not allocate memory for Symbol List\n"); return NULL;
    }
    newList->count=0;
    newList->head=NULL; newList->tail=NULL;
    return newList;
}

void insertSymbolListNode(symbolList* symList, symbolListNode* symListNode){
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
    pTreeNode* newTreeNode = (pTreeNode*) malloc(sizeof(pTreeNode));
    if(!newTreeNode){
        printf("Couldnt allocate memory for creating parse tree node\n"); return NULL;
    }
    newTreeNode->children = (pTreeNode**) malloc(INITIAL_CHILDREN_CAPACITY*sizeof(pTreeNode*));
    if(!(newTreeNode->children)){
        printf("Couldnt allocate memory for parse tree node's children\n"); return newTreeNode;
    }
    newTreeNode->capacity = INITIAL_CHILDREN_CAPACITY;
    newTreeNode->size = 0;
    newTreeNode->symbol = NULL;
    for(int i=0; i<INITIAL_CHILDREN_CAPACITY; i++) newTreeNode->children[i] = NULL;
    
    return newTreeNode;
}

pTree* createPTree(){
    pTree* newTree = (pTree*) malloc(sizeof(pTree));
    if(!newTree){
        printf("Could not allocate memory for creating parse tree\n"); return NULL;
    }
    newTree->root = createPTreeNode();
    return newTree;
}

void insertNodeAsChild(pTreeNode* par, pTreeNode* child){
    if(!par){
        printf("Parent node is null. Couldnt add child\n"); return;
    }
    if(!child){
        printf("Child node is null. Couldnt add child\n"); return;
    }
    if(par->size == par->capacity){
        par->capacity *= 2;
        par->children = (pTreeNode**) realloc(par->children, par->capacity*sizeof(pTreeNode*)); 
        if(!(par->children)){
            printf("Could not allocate memory for resizing node's children capacity\n"); return;
        }
    }
    par->children[(par->size)++] = child;
}

//FirstAndFollow computeFirstAndFollowSets(grammar G) {
    /*
    This function takes as input the grammar G, computes FIRST and FOLLOW information and populates appropriate data structure FirstAndFollow.
    First and Follow set automation must be attempted, keeping in view the programming confidence of the team members and the available time with the teams.
    If teams opt not to develop the module for computation of First and follow sets, the same can be computed manually and information be populated in the data structure appropriately.
    However, all members of the team must understand that any new grammar rule for any new construct will then require their expertise in computing FIRST and FOLLOW sets manually.
    */

//}

//void createParseTable(FirstAndFollow F, table T) {
    /*
    This function takes as input the FIRST and FOLLOW  information in F to populate the table T appropriately.
    */

//}

// void parseInputSourceCode(char* testcaseFile, table T) {
    /*
    This function takes as input the source code file and parses using the rules as per the predictive parse table T and returns a parse tree.
    The function gets the tokens using lexical analysis interface and establishes the syntactic structure of the input source code using rules in T.
    The function must report all errors appropriately (with line numbers) if the source code is syntactically incorrect.
    If the source code is correct then the token and all its relevant information is added to the parse tree.
    The start symbol of the grammar is the root of the parse tree and the tree grows as the syntax analysis moves in top down way.
    The function must display a message "Input source code is syntactically correct..........." for successful parsing.
    */

// }

//void printParseTree(parseTree PT, char* outfile) {
    /*
    This function provides an interface for observing the correctness of the creation of parse tree. The function prints the parse tree in inorder in the file outfile. The output is such that each line of the file outfile must contain the information corresponding to the currently visited node of the parse tree in the following format:

    lexeme CurrentNode lineno tokenName valueIfNumber parentNodeSymbol isLeafNode(yes/no) NodeSymbol

    The lexeme of the current node is printed when it is the leaf node else a dummy string of characters "‐‐‐‐" is printed.
    The line number is one of the information collected by the lexical analyzer during single pass of the source code.
    The token name corresponding to the current node is printed third.
    If the lexeme is an integer or real number, then its value computed by the lexical analyzer should be printed at the fourth place.
    Print the grammar symbol (non-terminal symbol) of the parent node of the currently visited node appropriately at fifth place (for the root node print ROOT for parent symbol).
    The sixth column is for printing yes or no appropriately.
    Print the non-terminal symbol of the node being currently visited at the 7th place, if the node is not the leaf node [Print the actual non-terminal symbol and not the enumerated values for the non-terminal].
    Ensure appropriate justification so that the columns appear neat and straight.
    */

//}

void initializeNonTerminalToString(){
    for(int i=0; i<NT_NOT_FOUND; i++){
        nonTerminalToString[i]=malloc(NON_TERM_LENGTH);
    }
    // char prg[NON_TERM_LENGTH] = "<program>"; 
    nonTerminalToString[program] = "<program>";
    // char othFn[NON_TERM_LENGTH] = "<otherFunctions>"; 
    nonTerminalToString[otherFunctions] = "<otherFunctions>";
    // char mfn[NON_TERM_LENGTH] = "<mainFunction>"; 
    nonTerminalToString[mainFunction] = "<mainFunction>";
    // char sms[NON_TERM_LENGTH] = "<stmts>"; 
    nonTerminalToString[stmts] = "<stmts>";
    // char sm[NON_TERM_LENGTH] = "<stmt>"; 
    nonTerminalToString[stmt] = "<stmt>";
    // char fn[NON_TERM_LENGTH] = "<function>"; 
    nonTerminalToString[function] = "<function>";
    // char ipr[NON_TERM_LENGTH] = "<input_par>"; 
    nonTerminalToString[input_par] = "<input_par>";
    // char opr[NON_TERM_LENGTH] = "<output_par>"; 
    nonTerminalToString[output_par] = "<output_par>";
    // char prmls[NON_TERM_LENGTH] = "<parameter_list>"; 
    nonTerminalToString[parameter_list] = "<parameter_list>";
    // char dtyp[NON_TERM_LENGTH] = "<dataType>"; 
    nonTerminalToString[dataType] = "<dataType>";
    // char rmlst[NON_TERM_LENGTH] = "<remaining_list>"; 
    nonTerminalToString[remaining_list] = "<remaining_list>";
    // char prmdtyp[NON_TERM_LENGTH] = "<primitiveDatatype>"; 
    nonTerminalToString[primitiveDatatype] = "<primitiveDatatype>";
    // char cnsdtyp[NON_TERM_LENGTH] = "<constructedDatatype>"; 
    nonTerminalToString[constructedDatatype] = "<constructedDatatype>";
    // char tpdfns[NON_TERM_LENGTH] = "<typeDefinitions>"; 
    nonTerminalToString[typeDefinitions] = "<typeDefinitions>"; 
    // char acorredf[NON_TERM_LENGTH] = "<actualOrRedefined>"; 
    nonTerminalToString[actualOrRedefined] = "<actualOrRedefined>"; 
    // char tpdfn[NON_TERM_LENGTH] = "<typeDefinition>"; 
    nonTerminalToString[typeDefinition] = "<typeDefinition>"; 
    // char dclns[NON_TERM_LENGTH] = "<declarations>"; 
    nonTerminalToString[declarations] = "<declarations>"; 
    // char dcln[NON_TERM_LENGTH] = "<declaration>"; 
    nonTerminalToString[declaration] = "<declaration>"; 
    // char othsms[NON_TERM_LENGTH] = "<otherStmts>"; 
    nonTerminalToString[otherStmts] = "<otherStmts>"; 
    // char rtrnst[NON_TERM_LENGTH] = "<returnStmt>"; 
    nonTerminalToString[returnStmt] = "<returnStmt>"; 
    // char dftypst[NON_TERM_LENGTH] = "<definetypestmt>"; 
    nonTerminalToString[definetypestmt] = "<definetypestmt>";
    // char flddef[NON_TERM_LENGTH] = "<fieldDefinition>"; 
    nonTerminalToString[fieldDefinition] = "<fieldDefinition>";
    // char fldtyp[NON_TERM_LENGTH] = "<fieldType>"; 
    nonTerminalToString[fieldType] = "<fieldType>"; 
    // char flddefns[NON_TERM_LENGTH] = "<fieldDefinitions>"; 
    nonTerminalToString[fieldDefinitions] = "<fieldDefinitions>";
    // char mrflds[NON_TERM_LENGTH] = "<moreFields>"; 
    nonTerminalToString[moreFields] = "<moreFields>"; 
    // char gon[NON_TERM_LENGTH] = "<global_or_not>"; 
    nonTerminalToString[global_or_not] = "<global_or_not>"; 
    // char asgnsm[NON_TERM_LENGTH] = "<assignmentStmt>"; 
    nonTerminalToString[assignmentStmt] = "<assignmentStmt>"; 
    // char itrsm[NON_TERM_LENGTH] = "<iterativeStmt>"; 
    nonTerminalToString[iterativeStmt] = "<iterativeStmt>"; 
    // char cndsm[NON_TERM_LENGTH] = "<conditionalStmt>"; 
    nonTerminalToString[conditionalStmt] = "<conditionalStmt>"; 
    // char elpr[NON_TERM_LENGTH] = "<elsePart>"; 
    nonTerminalToString[elsePart] = "<elsePart>"; 
    // char iosm[NON_TERM_LENGTH] = "<ioStmt>"; 
    nonTerminalToString[ioStmt] = "<ioStmt>"; 
    // char fnclsm[NON_TERM_LENGTH] = "<funCallStmt>"; 
    nonTerminalToString[funCallStmt] = "<funCallStmt>";
    // char sngorrec[NON_TERM_LENGTH] = "<SingleOrRecId>"; 
    nonTerminalToString[SingleOrRecId] = "<singleOrRecId>";
    // char optsngl[NON_TERM_LENGTH] = "<option_single_constructed>"; 
    nonTerminalToString[option_single_constructed] = "<option_single_constructed>"; 
    // char oneexp[NON_TERM_LENGTH] = "<oneExpansion>"; 
    nonTerminalToString[oneExpansion] = "<oneExpansion>"; 
    // char mrexp[NON_TERM_LENGTH] = "<moreExpansions>"; 
    nonTerminalToString[moreExpansions] = "<moreExpansions>"; 
    // char arthexpr[NON_TERM_LENGTH] = "<arithmeticExpression>"; 
    nonTerminalToString[arithmeticExpression] = "<arithmeticExpression>";
    // char exprm[NON_TERM_LENGTH] = "<expPrime>"; 
    nonTerminalToString[expPrime] = "<expPrime>"; 
    // char trm[NON_TERM_LENGTH] = "<term>"; 
    nonTerminalToString[term] = "<term>"; 
    // char trmprm[NON_TERM_LENGTH] = "<termPrime>"; 
    nonTerminalToString[termPrime] = "<termPrime>";
    // char fctr[NON_TERM_LENGTH] = "<factor>"; 
    nonTerminalToString[factor] = "<factor>"; 
    // char hghprc[NON_TERM_LENGTH] = "<highPrecedenceOperators>"; 
    nonTerminalToString[highPrecedenceOperators] = "<highPrecedenceOperators>";
    // char lowprc[NON_TERM_LENGTH] = "<lowPrecedenceOperators>"; 
    nonTerminalToString[lowPrecedenceOperators] = "<lowPrecedenceOperators>"; 
    // char otptprm[NON_TERM_LENGTH] = "<outputParameters>"; 
    nonTerminalToString[outputParameters] = "<outputParameters>"; 
    // char inptprm[NON_TERM_LENGTH] = "<inputParameters>"; 
    nonTerminalToString[inputParameters] = "<inputParameters>"; 
    // char idlst[NON_TERM_LENGTH] = "<idList>"; 
    nonTerminalToString[idList] = "<idList>"; 
    // char blexpr[NON_TERM_LENGTH] = "<booleanExpression>"; 
    nonTerminalToString[booleanExpression] = "<booleanExpression>";
    // char vr[NON_TERM_LENGTH] = "<var>"; 
    nonTerminalToString[var] = "<var>";  
    // char lgclop[NON_TERM_LENGTH] = "<logicalOp>"; 
    nonTerminalToString[logicalOp] = "<logicalOp>";
    // char rlop[NON_TERM_LENGTH] = "<relationalOp>"; 
    nonTerminalToString[relationalOp] = "<relationalOp>"; 
    // char optnrtn[NON_TERM_LENGTH] = "<optionalReturn>"; 
    nonTerminalToString[optionalReturn] = "<optionalReturn>"; 
    // char mrids[NON_TERM_LENGTH] = "<more_ids>"; 
    nonTerminalToString[more_ids] = "<more_ids>"; 
    // char thnta[NON_TERM_LENGTH] = "<A>"; 
    nonTerminalToString[A] = "<A>"; 
}

Token getTokenFromString(char* str){
    
    for(int i=0; i<TK_NOT_FOUND; i++){
        if(!strcmp(tokenToString[i], str))  return (Token)i;
    }
    return TK_NOT_FOUND;
}

NonTerminal getNonTerminalFromString(char* str){
    for(int i=0; i<NT_NOT_FOUND; i++){
        if(!strcmp(nonTerminalToString[i], str)) return  (NonTerminal)i;
    }
    return NT_NOT_FOUND;
}

void readGrammar(){
    char buff[MAX_RULE_LENGTH];
    FILE* fp = fopen("./src/parser/grammar.txt", "r");
    if(!fp){
        printf("Could not open grammar file\n"); return;
    }
    char* oneTok;
    for(; fgets(buff, MAX_RULE_LENGTH, fp); ){
        oneTok = strtok(buff, " \t\n\r");

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
                strcat(tmp, oneTok);
                gsym->isNonTerminal = false;
                gsym->tOrNt.t = getTokenFromString(tmp);
            }
            symbolListNode* rhsNode = createSymbolListNode(gsym);
            insertSymbolListNode(gRule->rhs, rhsNode);

            oneTok = strtok(NULL, " \t\n\r");
        }
        Grammar[numOfRules++] = gRule;
    }
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
    if(!(lst->tail)){
        lst->head = nd;
        lst->tail = nd;
    }
    else{
        lst->tail->next = nd;
        lst->tail = nd;
    }
}

void readFirst(){
    FILE* fp = fopen("./src/parser/FIRST.txt", "r");
    if(!fp){
        printf("Could not open First set file\n");
        return;
    }
    char buff[MAX_RULE_LENGTH];
    char* oneTok;

    for(int i=0; fgets(buff, MAX_RULE_LENGTH, fp); i++){  
        oneTok = strtok(buff, " \t\n\r");
        NonTerminal nt = getNonTerminalFromString(oneTok);
        // printf("%dhh %d\n", i, nt); fflush(stdout);

        for(; oneTok=strtok(NULL, " \t\n\r"); ){
            ffRhsNode* newNode = (ffRhsNode*) malloc(sizeof(ffRhsNode));
            if(!newNode){
                printf("Could not allocate memory for First set item\n");
                return;
            }
            newNode->next = NULL;
            newNode->tk = getTokenFromString(oneTok);
            insertFfRhsNode(First[nt], newNode);
        }
    }
}

void readFollow(){
    FILE* fp = fopen("./src/parser/FOLLOW.txt", "r");
    if(!fp){
        printf("Could not open Follow set file\n");
        return;
    }
    char buff[MAX_RULE_LENGTH];
    char* oneTok;

    for(; fgets(buff, MAX_RULE_LENGTH, fp); ){
        oneTok = strtok(buff, " \t\n\r");
        NonTerminal nt = getNonTerminalFromString(oneTok);

        for(; oneTok=strtok(NULL, " \t\n\r"); ){
            ffRhsNode* newNode = (ffRhsNode*) malloc(sizeof(ffRhsNode));
            if(!newNode){
                printf("Could not allocate memory for Follow set item\n");
                return;
            }
            newNode->next = NULL;
            newNode->tk = getTokenFromString(oneTok);
            insertFfRhsNode(Follow[nt], newNode);
        }
    }
}

void initializeAndReadFirstAndFollow(){
    First = (ffRhs**) malloc(NT_NOT_FOUND*sizeof(ffRhs*));
    Follow = (ffRhs**) malloc(NT_NOT_FOUND*sizeof(ffRhs*));
    if(!First || !Follow){
        printf("Could not allocate memory for first and follow arrays\n");
        return;
    }

    for(int i=0; i<NT_NOT_FOUND; i++){
        First[i] = (ffRhs*) malloc(sizeof(ffRhs));
        if(!(First[i])){
            printf("Could not allocate memory for first sets initialization\n");
            return;
        }
        First[i]->head = NULL; First[i]->tail = NULL;

        Follow[i] = (ffRhs*) malloc(sizeof(ffRhs));
        if(!(Follow[i])){
            printf("Could not allocate memory for follow sets initialization\n");
            return;
        }
        Follow[i]->head = NULL; Follow[i]->tail = NULL;
    }
    readFirst();
    readFollow();

}

void printFirstAndFollow(){
    FILE* foutFirst = fopen("readfrst.txt", "w");
    for(int i=0; i<NT_NOT_FOUND; i++){
        fprintf(foutFirst, "%s:\t", nonTerminalToString[i]);
        ffRhsNode* tmp = First[i]->head;
        for(; tmp; tmp=tmp->next){
            fprintf(foutFirst, "%s\t", tokenToString[tmp->tk]);
        }
        fprintf(foutFirst, "\n");
    }
    fclose(foutFirst);

    FILE* foutFollow = fopen("readfllw.txt", "w");
    for(int i=0; i<NT_NOT_FOUND; i++){
        fprintf(foutFollow, "%s:\t", nonTerminalToString[i]);
        ffRhsNode* tmp = Follow[i]->head;
        for(; tmp; tmp=tmp->next){
            fprintf(foutFollow, "%s\t", tokenToString[tmp->tk]);
        }
        fprintf(foutFollow, "\n");
    }
    fclose(foutFollow);
    
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
    if(!lst) return false;
    ffRhsNode* current = lst->head;
    for(; current; current=current->next){
        if(current->tk==tmpkey) return true;
    }
    return false;
}

ffRhs* getFirstOfRhs(symbolList* grhs){
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
    if(!rl) return;
    ffRhsNode *tmp, *tn;
    tmp = rl->head;
    for(; tmp; ){
        tn = tmp;
        tmp=tmp->next;
        free(tn);
    }
}

void addRulesToParseTable(){
    // printf("\nAdding rules to parse table\n");
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

pTree* parseTokens(linkedList* tokensFromLexer, FILE* foutP, bool* hasSyntaxError){
    if(!tokensFromLexer){
        printf("Tokens list from lexer is null. Parsing failed\n"); return NULL;
    }
    printf("Parsing...\n"); fflush(stdout);
    tokenInfo* inputPtr = tokensFromLexer->head;
    pTree* theParseTree = createPTree();
    pTreeNode* currentNode = theParseTree->root;
    grammarSymbol* gs = (grammarSymbol*) malloc(sizeof(grammarSymbol));
    gs->isNonTerminal = true; gs->tOrNt.nt = program;
    currentNode->symbol = gs;
    Stack* theStack = createNewStack();
    push(theStack, currentNode);

    printf("Parsing starting...\n"); fflush(stdout);
    for(int pritr=0 ;!isEmpty(theStack) && inputPtr; currentNode=top(theStack), pritr++){
        // printf("Parse iter %d...\n", pritr); fflush(stdout);
        if(inputPtr->STE->tokenType==COMMENT || inputPtr->STE->tokenType>=LEXICAL_ERROR){
            inputPtr = inputPtr->next;
            continue;
        }
        if(!(currentNode->symbol->isNonTerminal) && currentNode->symbol->tOrNt.t==EPS){
            pop(theStack); 
            continue;
        }
        if(!(currentNode->symbol->isNonTerminal) && currentNode->symbol->tOrNt.t==inputPtr->STE->tokenType){
            // printf("cs 1...\n"); fflush(stdout);
            pop(theStack); inputPtr = inputPtr->next;
        }
        else if(!(currentNode->symbol->isNonTerminal)){
            // printf("cs 2...\n"); fflush(stdout);
            // Error; handle mismatch
            *hasSyntaxError = true;
            fprintf(foutP, "\nSyntax error in line %d\n", inputPtr->lineNumber);
            fprintf(foutP, "stack top: %s, input ptr: %s\n", tokenToString[currentNode->symbol->tOrNt.t], tokenToString[inputPtr->STE->tokenType]);
            // inputPtr = inputPtr->next;
            pop(theStack);
        }
        else if(parseTable[currentNode->symbol->tOrNt.nt][inputPtr->STE->tokenType]==NULL){
            // printf("cs 3...\n"); fflush(stdout);
            // Error; handle blank/error entry of parse table
            *hasSyntaxError = true;
            fprintf(foutP, "\nSyntax error in line %d\n", inputPtr->lineNumber);
            fprintf(foutP, "stack top: %s, input ptr: %s\n", nonTerminalToString[currentNode->symbol->tOrNt.nt], tokenToString[inputPtr->STE->tokenType]);

            if(existsInRhs(AutoFollow[currentNode->symbol->tOrNt.nt],  inputPtr->STE->tokenType))
                pop(theStack);
            else 
                inputPtr=inputPtr->next;
        }
        else{
            // printf("cs 4...\n"); fflush(stdout);
            grammarRule* tmpRule = parseTable[currentNode->symbol->tOrNt.nt][inputPtr->STE->tokenType];
            fprintf(foutP, "\nReducing using the rule: \n"); 
            printRule(tmpRule, foutP);
            pop(theStack);
            symbolListNode* trhItr = tmpRule->rhs->head;
            pTreeNode* pn;
            for(; trhItr; trhItr=trhItr->next){
                pn = createPTreeNode();
                pn->symbol = (grammarSymbol*) malloc(sizeof(grammarSymbol));
                pn->symbol->isNonTerminal = trhItr->symb->isNonTerminal;
                if(pn->symbol->isNonTerminal) pn->symbol->tOrNt.nt = trhItr->symb->tOrNt.nt;
                else pn->symbol->tOrNt.t = trhItr->symb->tOrNt.t;
                insertNodeAsChild(currentNode, pn);
            }
            for(int chi=currentNode->size-1; chi>=0; chi--){
                push(theStack, currentNode->children[chi]);
            }
        }
    }
    if(!(*hasSyntaxError)){
        printf("\nNo syntax errors. Parsing successful!\n");
    }
    else{
        printf("\nehh\n");
    }
    return theParseTree;
}

void computeFirstSets(){
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
            }
        }
    }
}

void initializeAndComputeFirstAndFollow(){
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

int main(){

    FILE* fp = fopen("./TestCases/t6.txt", "r");
    if(!fp){
        printf("Could not open file input file for parsing\n");
        return 0;
    }
    linkedList* tokensFromLexer = LexInput(fp);

    initializeNonTerminalToString();
    readGrammar();
    // printGrammar();

    // initializeAndReadFirstAndFollow();
    // printFirstAndFollow();
    initializeAndComputeFirstAndFollow();
    printComputedFirstAndFollow();

    initializeParseTable();
    // printParseTable();

    FILE* fpout = fopen("ParserOutput.txt", "w");
    if(!fpout){
        printf("Could not open file for parser output\n");
        return 0;
    }

    bool hasSyntaxError = false;
    pTree* parseTree = parseTokens(tokensFromLexer, fpout, &hasSyntaxError);
    
}