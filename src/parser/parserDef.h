/*
#                    GROUP - 8
# 2020B1A70630P                       Aditya Thakur
# 2021A7PS2001P                       Amal Sayeed
# 2020A7PS2005P                       Ohiduz Zaman
# 2020A7PS2682P                       Priyansh Patel
# 2020A7PS2002P                       Rachoita Das
# 2020B1A70611P                       Subhramit Basu Bhowmick
*/

// Contains all definitions for data types such as grammar, table, parseTree etc. used in parser.c

#ifndef PARSER_DEF
#define PARSER_DEF

#include "../lexer/lexerDef.h"

#define NON_TERM_LENGTH 30
#define MAX_GRAMMAR_RULES 100
#define MAX_RULE_LENGTH 512
#define INITIAL_CHILDREN_CAPACITY 10

typedef enum NonTerminal{
    program,
    otherFunctions,
    mainFunction,
    stmts,
    stmt,
    function,
    input_par,
    output_par,
    parameter_list,
    dataType,
    remaining_list,
    primitiveDatatype,
    constructedDatatype,
    typeDefinitions,
    actualOrRedefined,
    typeDefinition,
    declarations,
    declaration,
    otherStmts,
    returnStmt,
    definetypestmt,
    fieldDefinition,
    fieldType,
    fieldDefinitions, 
    moreFields,
    global_or_not,
    assignmentStmt,
    iterativeStmt,
    conditionalStmt,
    elsePart,
    ioStmt,
    funCallStmt,
    SingleOrRecId,
    option_single_constructed,
    oneExpansion,
    moreExpansions,
    arithmeticExpression,
    expPrime,
    term,
    termPrime,
    factor,
    highPrecedenceOperators,
    lowPrecedenceOperators,
    outputParameters,
    inputParameters,
    idList,
    booleanExpression,
    var,
    logicalOp,
    relationalOp,
    optionalReturn,
    more_ids,
    A,
    NT_NOT_FOUND
} NonTerminal;

char* nonTerminalToString[NT_NOT_FOUND];

typedef struct grammarSymbol{
    bool isNonTerminal;
    union{
        NonTerminal nt;
        Token t;
    } tOrNt;
} grammarSymbol;

typedef struct symbolListNode {
    grammarSymbol* symb;
    struct symbolListNode* prev;
    struct symbolListNode* next;
} symbolListNode;

typedef struct symbolList{
    symbolListNode* head;
    symbolListNode* tail;
    int count;
} symbolList;

typedef struct grammarRule{
    grammarSymbol* lhs;
    symbolList* rhs;
} grammarRule;

grammarRule* Grammar[MAX_GRAMMAR_RULES];
int  numOfRules = 0;

typedef struct ffRhsNode{
    Token tk;
    struct ffRhsNode* next;
} ffRhsNode;

typedef struct ffRhs{
    ffRhsNode* head;
    ffRhsNode* tail;
} ffRhs;


ffRhs** First;
ffRhs** Follow;
ffRhs** AutoFirst;
ffRhs** AutoFollow;

grammarRule*** parseTable;

typedef struct pTreeNode{
    grammarSymbol* symbol;
    struct pTreeNode** children;
    int capacity, size;
} pTreeNode;

typedef struct pTree{
    pTreeNode* root;
} pTree;

#endif