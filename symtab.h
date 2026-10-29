#ifndef SYMTAB_H
#define SYMTAB_H

/* =====================================================================
 * symtab.h  --  Symbol Table (Part C)
 *
 * The symbol table remembers every variable the program declares. Each
 * entry stores exactly the fields the sheet lists:
 *    - identifier name
 *    - data type
 *    - scope level / scope reference
 *    - line number of declaration
 *    - initialization status (initialized or not)
 *
 * SCOPES. The language has nested scopes created by { } blocks:
 *    level 0  = global (outside any block)
 *    level 1  = inside the first { }
 *    level 2  = inside a { } that is inside another, etc.
 * We model this as a STACK of scopes. Entering a block pushes a new scope;
 * leaving it pops that scope (its variables die). A name is looked up from
 * the innermost scope outward, so an inner variable can shadow an outer one.
 * ===================================================================== */

#include "ast.h"   /* for DataType */

/* One row of the symbol table. */
typedef struct Symbol {
    char    *name;          /* identifier name                       */
    DataType type;          /* its data type                         */
    int      scope_level;   /* 0 = global, 1,2,... = nested          */
    int      decl_line;     /* line where it was declared            */
    int      initialized;   /* 1 if given a value, 0 if not          */
    struct Symbol *next;    /* next symbol in the SAME scope (linked list) */
} Symbol;

/* One scope = a list of the symbols declared directly in it. */
typedef struct Scope {
    int     level;
    Symbol *symbols;        /* head of this scope's symbol list      */
    struct Scope *parent;   /* enclosing scope (NULL for global)     */
} Scope;

/* ---- Scope stack operations ---- */
void    initSymtab(void);          /* create the global scope (level 0) */
void    enterScope(void);          /* push a new, deeper scope          */
void    exitScope(void);           /* pop the current scope             */
int     currentLevel(void);        /* level of the current scope        */

/* ---- Symbol operations ----
 * insertSymbol: add a declaration to the CURRENT scope.
 *   Returns the new Symbol*, or NULL if a symbol with that name already
 *   exists in the current scope (a duplicate declaration).
 * lookupSymbol: search from the current scope outward through parents.
 *   Returns the Symbol* if found anywhere in scope, else NULL.
 * lookupCurrentScope: search ONLY the current scope (used for dup check).
 */
Symbol *insertSymbol(const char *name, DataType type, int line, int initialized);
Symbol *lookupSymbol(const char *name);
Symbol *lookupCurrentScope(const char *name);

/* Dump the whole table (for the demo / report screenshots). */
void    printSymtab(void);

#endif /* SYMTAB_H */
