#ifndef POST_PARSER_COMPILER_H
#define POST_PARSER_COMPILER_H

#include <stdio.h>
#include "../parser/parserDef.h"

/* Borrow tree; emit native-host x86-64 GAS Intel assembly to output.
 * Link the result with the prebuilt runtime object. _WIN32 selects Win64 COFF;
 * other x86-64 hosts select SysV ELF. Non-x86-64 compiler builds are rejected.
 * Returns zero on success, nonzero on a semantic, allocation, or output error.
 * Diagnostics go to stderr. No output is written before semantic validation.
 * The caller owns both tree and output; success flushes but does not close it.
 * No frontend global state is modified.
 *
 * Generated code uses int64_t/double and checked arithmetic, input, and union
 * accesses. Primitive operands must match exactly. Record/scalar operations
 * apply recursively; real results stored in integer fields truncate toward
 * zero with range checks. Globals start at zero; local reads and function
 * outputs require definite initialization (conservative branch/loop analysis).
 *
 * Record I/O visits primitive leaves, one output value per line. Variant I/O
 * visits tagvalue first, then remaining fields in declaration order, selecting
 * only the active union member. Tags are 1-based integral int/real values.
 * Copying a union field checks both its source tag and destination tag.
 * A union must be a direct record field with one primitive sibling tagvalue;
 * standalone unions, multiple unions sharing a tag, unions containing unions
 * (including through records), and variant arithmetic are rejected explicitly.
 * Semantic/type nesting is limited to 256 and initialization slots to 1048576
 * per type/function; oversized inputs receive diagnostics, not partial code.
 */
int emitAssemblyProgram(pTree *tree, FILE *output);

/* Perform the same parser normalization, semantic/type and initialization
 * checks without generating output or invoking the assembler/runtime. */
int checkProgram(pTree *tree);

#endif
