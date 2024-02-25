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
#include "parser.h"
#include "parserDef.h"


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
    for(int i=0; fgets(buff, MAX_RULE_LENGTH, fp); i++){
        // printf("itr %d\t", i);
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
        // printf("%s\t", oneTok);
        gRule->lhs->tOrNt.nt = getNonTerminalFromString(oneTok);
        // printf("%s\t", nonTerminalToString[gRule->lhs->tOrNt.nt]);
        gRule->rhs = createSymbolList();

        oneTok = strtok(NULL, " \t\n\r");

        for(; oneTok; ){
            // printf("%s\t", oneTok);
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
        // printf("\n");
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

void initializeTokenToStringFP(){
    for(int i=0; i<TK_NOT_FOUND; i++){
        tokenToString[i] = malloc(TOKEN_NAME_LENGTH);
    }
    // char asgn[TOKEN_NAME_LENGTH] = "TK_ASSIGNOP"; 
    tokenToString[ASSIGNOP] = "TK_ASSIGNOP";
    // char cmnt[TOKEN_NAME_LENGTH] = "TK_COMMENT"; 
    tokenToString[COMMENT] = "TK_COMMENT"; 
    // char fdid[TOKEN_NAME_LENGTH] = "TK_FIELDID"; 
    tokenToString[FIELDID] = "TK_FIELDID"; 
    // char id[TOKEN_NAME_LENGTH] = "TK_ID"; 
    tokenToString[ID] = "TK_ID"; 
    // char num[TOKEN_NAME_LENGTH] = "TK_NUM"; 
    tokenToString[NUM] = "TK_NUM"; 
    // char rnum[TOKEN_NAME_LENGTH] = "TK_RNUM"; 
    tokenToString[RNUM] = "TK_RNUM"; 
    // char fnid[TOKEN_NAME_LENGTH] = "TK_FUNID"; 
    tokenToString[FUNID] = "TK_FUNID"; 
    // char ruid[TOKEN_NAME_LENGTH] = "TK_RUID"; 
    tokenToString[RUID] = "TK_RUID"; 
    // char with[TOKEN_NAME_LENGTH] = "TK_WITH"; 
    tokenToString[WITH] = "TK_WITH"; 
    // char pmtrs[TOKEN_NAME_LENGTH] = "TK_PARAMETERS"; 
    tokenToString[PARAMETERS] = "TK_PARAMETERS";
    // char end[TOKEN_NAME_LENGTH] = "TK_END"; 
    tokenToString[END] = "TK_END"; 
    // char whle[TOKEN_NAME_LENGTH] = "TK_WHILE"; 
    tokenToString[WHILE] = "TK_WHILE";
    // char unon[TOKEN_NAME_LENGTH] = "TK_UNION"; 
    tokenToString[UNION] = "TK_UNION";
    // char eunon[TOKEN_NAME_LENGTH] = "TK_ENDUNION"; 
    tokenToString[ENDUNION] = "TK_ENDUNION";
    // char dftp[TOKEN_NAME_LENGTH] = "TK_DEFINETYPE"; 
    tokenToString[DEFINETYPE] = "TK_DEFINETYPE";
    // char as[TOKEN_NAME_LENGTH] = "TK_AS"; 
    tokenToString[AS] = "TK_AS";
    // char type[TOKEN_NAME_LENGTH] = "TK_TYPE"; 
    tokenToString[TYPE] = "TK_TYPE";
    // char mainf[TOKEN_NAME_LENGTH] = "TK_MAIN"; 
    tokenToString[MAIN] = "TK_MAIN";
    // char glbl[TOKEN_NAME_LENGTH] = "TK_GLOBAL"; 
    tokenToString[GLOBAL] = "TK_GLOBAL";
    // char pmtr[TOKEN_NAME_LENGTH] = "TK_PARAMETER"; 
    tokenToString[PARAMETER] = "TK_PARAMETER";
    // char list[TOKEN_NAME_LENGTH] = "TK_LIST"; 
    tokenToString[LIST] = "TK_LIST"; 
    // char sql[TOKEN_NAME_LENGTH] = "TK_SQL"; 
    tokenToString[SQL] = "TK_SQL";
    // char sqr[TOKEN_NAME_LENGTH] = "TK_SQR"; 
    tokenToString[SQR] = "TK_SQR";
    // char inpt[TOKEN_NAME_LENGTH] = "TK_INPUT"; 
    tokenToString[INPUT] = "TK_INPUT";
    // char otpt[TOKEN_NAME_LENGTH] = "TK_OUTPUT"; 
    tokenToString[OUTPUT] = "TK_OUTPUT";
    // char dint[TOKEN_NAME_LENGTH] = "TK_INT"; 
    tokenToString[INT] = "TK_INT";
    // char dreal[TOKEN_NAME_LENGTH] = "TK_REAL"; 
    tokenToString[REAL] = "TK_REAL";
    // char cmma[TOKEN_NAME_LENGTH] = "TK_COMMA"; 
    tokenToString[COMMA] = "TK_COMMA";
    // char semc[TOKEN_NAME_LENGTH] = "TK_SEM"; 
    tokenToString[SEM] = "TK_SEM";
    // char ifcl[TOKEN_NAME_LENGTH] = "TK_IF"; 
    tokenToString[IF] = "TK_IF";
    // char clon[TOKEN_NAME_LENGTH] = "TK_COLON"; 
    tokenToString[COLON] = "TK_COLON";
    // char dot[TOKEN_NAME_LENGTH] = "TK_DOT"; 
    tokenToString[DOT] = "TK_DOT";
    // char ewhle[TOKEN_NAME_LENGTH] = "TK_ENDWHILE"; 
    tokenToString[ENDWHILE] = "TK_ENDWHILE";
    // char opp[TOKEN_NAME_LENGTH] = "TK_OP"; 
    tokenToString[OP] = "TK_OP";
    // char clp[TOKEN_NAME_LENGTH] = "TK_CL"; 
    tokenToString[CL] = "TK_CL";
    // char then[TOKEN_NAME_LENGTH] = "TK_THEN"; 
    tokenToString[THEN] = "TK_THEN";
    // char eif[TOKEN_NAME_LENGTH] = "TK_ENDIF"; 
    tokenToString[ENDIF] = "TK_ENDIF";
    // char read[TOKEN_NAME_LENGTH] = "TK_READ"; 
    tokenToString[READ] = "TK_READ";
    // char wrte[TOKEN_NAME_LENGTH] = "TK_WRITE"; 
    tokenToString[WRITE] = "TK_WRITE";
    // char rtrn[TOKEN_NAME_LENGTH] = "TK_RETURN"; 
    tokenToString[RETURN] = "TK_RETURN";
    // char plus[TOKEN_NAME_LENGTH] = "TK_PLUS"; 
    tokenToString[PLUS] = "TK_PLUS";
    // char minus[TOKEN_NAME_LENGTH] = "TK_MINUS"; 
    tokenToString[MINUS] = "TK_MINUS";
    // char mul[TOKEN_NAME_LENGTH] = "TK_MUL"; 
    tokenToString[MUL] = "TK_MUL";
    // char div[TOKEN_NAME_LENGTH] = "TK_DIV"; 
    tokenToString[DIV] = "TK_DIV";
    // char call[TOKEN_NAME_LENGTH] = "TK_CALL"; 
    tokenToString[CALL] = "TK_CALL";
    // char rcrd[TOKEN_NAME_LENGTH] = "TK_RECORD"; 
    tokenToString[RECORD] = "TK_RECORD";
    // char erec[TOKEN_NAME_LENGTH] = "TK_ENDRECORD"; 
    tokenToString[ENDRECORD] = "TK_ENDRECORD";
    // char elsecl[TOKEN_NAME_LENGTH] = "TK_ELSE"; 
    tokenToString[ELSE] = "TK_ELSE";
    // char and[TOKEN_NAME_LENGTH] = "TK_AND"; 
    tokenToString[AND] = "TK_AND";
    // char or[TOKEN_NAME_LENGTH] = "TK_OR"; 
    tokenToString[OR] = "TK_OR";
    // char not[TOKEN_NAME_LENGTH] = "TK_NOT"; 
    tokenToString[NOT] = "TK_NOT";
    // char lthan[TOKEN_NAME_LENGTH] = "TK_LT"; 
    tokenToString[LT] = "TK_LT";
    // char leq[TOKEN_NAME_LENGTH] = "TK_LE"; 
    tokenToString[LE] = "TK_LE";
    // char eql[TOKEN_NAME_LENGTH] = "TK_EQ"; 
    tokenToString[EQ] = "TK_EQ";
    // char gthan[TOKEN_NAME_LENGTH] = "TK_GT"; 
    tokenToString[GT] = "TK_GT";
    // char geq[TOKEN_NAME_LENGTH] = "TK_GE"; 
    tokenToString[GE] = "TK_GE";
    // char neq[TOKEN_NAME_LENGTH] = "TK_NE"; 
    tokenToString[NE] = "TK_NE";
    // char eps[TOKEN_NAME_LENGTH] = "TK_EPS"; 
    tokenToString[EPS] = "TK_EPS";
    // char dlr[TOKEN_NAME_LENGTH] = "TK_DOLLAR"; 
    tokenToString[DOLLAR] = "TK_DOLLAR";
    // char lxer[TOKEN_NAME_LENGTH] = "LEXICAL_ERROR"; 
    tokenToString[LEXICAL_ERROR] = "LEXICAL_ERROR";
}

int main(){
    
    initializeTokenToStringFP();
    initializeNonTerminalToString();
    // for(int i=0; i<TK_NOT_FOUND; i++) printf("%s\n", tokenToString[i]);
    // printf("---------------------------\n");
    // for(int i=0; i<NT_NOT_FOUND; i++) printf("%s\n", nonTerminalToString[i]);
    readGrammar();
    printGrammar();
}