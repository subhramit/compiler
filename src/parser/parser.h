/*
#                    GROUP - 8
# 2020B1A70630P                       Aditya Thakur
# 2021A7PS2001P                       Amal Sayeed
# 2021A7PS2005P                       Ohiduz Zaman
# 2021A7PS2682P                       Priyansh Patel
# 2021A7PS2002P                       Rachoita Das
# 2020B1A70611P                       Subhramit Basu Bhowmick
*/

// Contains function prototype declarations of functions in parser.c

#ifndef PARSER_PROTOTYPES
#define PARSER_PROTOTYPES

#include "../lexer/lexer.h"
#include "parserDef.h"


/* Parse a file path (not source text). Returns NULL on any frontend/file error.
 * Adds the number of errors to *errors when non-NULL; initialize it to zero
 * for a per-call count. Diagnostics are reported to stderr, independently of
 * shouldPrint. Grammar/tables and token STEs are cached globally: calls are
 * not thread-safe. Linkers must still supply the legacy bool shouldPrint.
 * Define COMPILER_ROOT as a quoted repository path for grammar lookup outside
 * the repository (cwd-relative grammar takes precedence).
 */
pTree* parseSourceFile(const char *input, int *errors);

/* Releases the tree, its nodes, symbols and child arrays. STE pointers are
 * borrowed (lexer globals or shared epsilon); consumers must not free them.
 * Trees/STE pointers remain valid across subsequent parses. NULL is accepted.
 */
void freeParseTree(pTree *tree);

void parseInputSourceCode(char* inpFile, char* opFile);

#endif