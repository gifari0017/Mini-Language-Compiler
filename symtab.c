/* =====================================================================
 * symtab.c  --  Symbol Table implementation (Part C)
 * ===================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* The current (innermost) scope. Scopes link to their parent, forming the
   scope stack. `current` is the top of that stack. */
static Scope *current = NULL;

/* A flat record of EVERY symbol ever inserted, kept only so we can print
   the full table at the end (even for scopes already popped). The semantic
   checks never use this list; they use the live scope stack above. */
#define MAX_ALL 1024
static Symbol *all_symbols[MAX_ALL];
static int     all_count = 0;

void initSymtab(void) {
    current = (Scope *)calloc(1, sizeof(Scope));
    current->level  = 0;        /* global scope */
    current->symbols = NULL;
    current->parent = NULL;
    all_count = 0;
}

void enterScope(void) {
    Scope *s = (Scope *)calloc(1, sizeof(Scope));
    s->level  = current->level + 1;
    s->parent = current;
    current   = s;              /* push */
}

void exitScope(void) {
    if (!current || !current->parent) return;  /* never pop global */
    Scope *dead = current;
    current = current->parent;  /* pop */
    /* We do NOT free the Symbol structs: all_symbols[] still points to them
       for the final printSymtab(). Only the Scope wrapper is freed. */
    free(dead);
}

int currentLevel(void) { return current ? current->level : 0; }

/* Search only the current scope's own list. */
Symbol *lookupCurrentScope(const char *name) {
    for (Symbol *s = current->symbols; s; s = s->next)
        if (strcmp(s->name, name) == 0) return s;
    return NULL;
}

/* Search current scope, then outward through all enclosing scopes. */
Symbol *lookupSymbol(const char *name) {
    for (Scope *sc = current; sc; sc = sc->parent)
        for (Symbol *s = sc->symbols; s; s = s->next)
            if (strcmp(s->name, name) == 0) return s;
    return NULL;
}

/* Insert into the current scope. Returns NULL if already declared here. */
Symbol *insertSymbol(const char *name, DataType type, int line, int initialized) {
    if (lookupCurrentScope(name)) return NULL;   /* duplicate in this scope */

    Symbol *s = (Symbol *)calloc(1, sizeof(Symbol));
    s->name        = strdup(name);
    s->type        = type;
    s->scope_level = current->level;
    s->decl_line   = line;
    s->initialized = initialized;

    /* link into current scope's list */
    s->next = current->symbols;
    current->symbols = s;

    /* record for final printing */
    if (all_count < MAX_ALL) all_symbols[all_count++] = s;
    return s;
}

void printSymtab(void) {
    printf("\n==================== SYMBOL TABLE ====================\n");
    printf("%-12s  %-8s  %-6s  %-9s  %s\n",
           "NAME", "TYPE", "SCOPE", "DECL_LINE", "INITIALIZED");
    printf("-----------------------------------------------------\n");
    for (int i = 0; i < all_count; i++) {
        Symbol *s = all_symbols[i];
        printf("%-12s  %-8s  %-6d  %-9d  %s\n",
               s->name, typeName(s->type), s->scope_level, s->decl_line,
               s->initialized ? "yes" : "no");
    }
    printf("=====================================================\n");
}
