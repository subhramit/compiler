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

//void parseInputSourceCode(char* testcaseFile, table T) {
    /*
    This function takes as input the source code file and parses using the rules as per the predictive parse table T and returns a parse tree.
    The function gets the tokens using lexical analysis interface and establishes the syntactic structure of the input source code using rules in T.
    The function must report all errors appropriately (with line numbers) if the source code is syntactically incorrect.
    If the source code is correct then the token and all its relevant information is added to the parse tree.
    The start symbol of the grammar is the root of the parse tree and the tree grows as the syntax analysis moves in top down way.
    The function must display a message "Input source code is syntactically correct..........." for successful parsing.
    */

//}

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
