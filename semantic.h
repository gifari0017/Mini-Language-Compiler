#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

/* Walk the AST, build the symbol table, and report semantic errors.
   Returns the number of semantic errors found. */
int analyze(Node *root);

extern int semantic_error_count;

#endif /* SEMANTIC_H */
