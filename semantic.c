/* =====================================================================
 * semantic.c  --  Semantic Analysis (Part D)
 *
 * After the parser builds the AST, we walk it ONCE more. During the walk we:
 *   - build the symbol table (declarations -> rows; blocks -> scopes)
 *   - report semantic errors with line numbers
 *
 * Errors detected (required):
 *   a) undeclared variable usage
 *   b) duplicate declaration (same name, same scope)
 *   c) type mismatch in expressions
 * Bonus:
 *   d) assignment type mismatch
 *   e) scope violation (name exists, but not in any visible scope)
 *   f) uninitialized variable use
 *
 * TYPE COMPATIBILITY RULES (documented for the report):
 *   - int and float are "numeric".
 *   - Arithmetic (+ - * /): both operands numeric.  Result is float if
 *     either operand is float, otherwise int.
 *   - Modulo (%): both operands must be int.  Result int.
 *   - Relational (< <= > >=): both operands numeric.  Result bool.
 *   - Equality (== !=): operands must have the SAME category
 *     (numeric/numeric, bool/bool, or string/string).  Result bool.
 *   - Logical (&& || !): operands must be bool.  Result bool.
 *   - Assignment / initialization: same type, OR int value into a float
 *     variable (widening).  Everything else is a mismatch.
 * ===================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"
#include "symtab.h"
#include "parser.tab.h"   /* operator token codes: PLUS, EQ, AND, ... */

int semantic_error_count = 0;

/* Record of every name ever declared, so a failed in-scope lookup can tell
   "declared elsewhere -> scope violation" from "never declared -> undeclared". */
#define MAX_NAMES 1024
static const char *declared_names[MAX_NAMES];
static int declared_count = 0;
static int wasEverDeclared(const char *name) {
    for (int i = 0; i < declared_count; i++)
        if (declared_names[i] && strcmp(declared_names[i], name) == 0)
            return 1;
    return 0;
}
static void rememberName(const char *name) {
    if (declared_count < MAX_NAMES) declared_names[declared_count++] = name;
}

static void semError(int line, const char *fmt, const char *a, const char *b) {
    semantic_error_count++;
    fprintf(stderr, "Semantic Error at line %d: ", line);
    fprintf(stderr, fmt, a, b);
    fprintf(stderr, "\n");
}

static int isNumeric(DataType t) { return t == TYPE_INT || t == TYPE_FLOAT; }

/* Is `src` assignable to a variable declared as `dst`? */
static int assignable(DataType dst, DataType src) {
    if (src == TYPE_ERROR || dst == TYPE_ERROR) return 1;   /* avoid cascade */
    if (dst == src) return 1;
    if (dst == TYPE_FLOAT && src == TYPE_INT) return 1;     /* int -> float  */
    return 0;
}

/* Forward decls */
static DataType checkExpr(Node *n);
static void     checkStmt(Node *n);

/* ---- Type-check an expression; returns its result type ---- */
static DataType checkExpr(Node *n) {
    if (!n) return TYPE_ERROR;

    switch (n->kind) {
        case N_INT:    n->type = TYPE_INT;    return TYPE_INT;
        case N_FLOAT:  n->type = TYPE_FLOAT;  return TYPE_FLOAT;
        case N_STRING: n->type = TYPE_STRING; return TYPE_STRING;
        case N_BOOL:   n->type = TYPE_BOOL;   return TYPE_BOOL;

        case N_VAR: {
            Symbol *s = lookupSymbol(n->sval);
            if (!s) {
                if (wasEverDeclared(n->sval))
                    semError(n->line, "variable '%s' used outside its scope", n->sval, NULL);
                else
                    semError(n->line, "undeclared variable '%s'", n->sval, NULL);
                n->type = TYPE_ERROR;
                return TYPE_ERROR;
            }
            if (!s->initialized)
                semError(n->line, "variable '%s' used before initialization", n->sval, NULL);
            n->type = s->type;
            return s->type;
        }

        case N_UNOP: {
            DataType t = checkExpr(n->left);
            if (n->op == NOT) {
                if (t != TYPE_BOOL && t != TYPE_ERROR)
                    semError(n->line, "operator '!' requires a bool operand", NULL, NULL);
                n->type = TYPE_BOOL; return TYPE_BOOL;
            }
            /* unary minus */
            if (!isNumeric(t) && t != TYPE_ERROR)
                semError(n->line, "unary '-' requires a numeric operand", NULL, NULL);
            n->type = (t == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
            return n->type;
        }

        case N_BINOP: {
            DataType l = checkExpr(n->left);
            DataType r = checkExpr(n->right);
            int op = n->op;

            /* logical */
            if (op == AND || op == OR) {
                if ((l != TYPE_BOOL && l != TYPE_ERROR) ||
                    (r != TYPE_BOOL && r != TYPE_ERROR))
                    semError(n->line, "operator '%s' requires bool operands", opName(op), NULL);
                n->type = TYPE_BOOL; return TYPE_BOOL;
            }

            /* equality */
            if (op == EQ || op == NE) {
                int ok = (isNumeric(l) && isNumeric(r)) || (l == r);
                if (!ok && l != TYPE_ERROR && r != TYPE_ERROR)
                    semError(n->line, "type mismatch: cannot compare '%s' with '%s'",
                             typeName(l), typeName(r));
                n->type = TYPE_BOOL; return TYPE_BOOL;
            }

            /* relational */
            if (op == LT || op == LE || op == GT || op == GE) {
                if ((!isNumeric(l) || !isNumeric(r)) && l != TYPE_ERROR && r != TYPE_ERROR)
                    semError(n->line, "type mismatch: cannot compare '%s' with '%s'",
                             typeName(l), typeName(r));
                n->type = TYPE_BOOL; return TYPE_BOOL;
            }

            /* modulo: int only */
            if (op == MOD) {
                if ((l != TYPE_INT || r != TYPE_INT) && l != TYPE_ERROR && r != TYPE_ERROR)
                    semError(n->line, "operator '%%' requires int operands", NULL, NULL);
                n->type = TYPE_INT; return TYPE_INT;
            }

            /* arithmetic + - * / */
            if (op == PLUS || op == MINUS || op == MUL || op == DIV) {
                if ((!isNumeric(l) || !isNumeric(r)) && l != TYPE_ERROR && r != TYPE_ERROR) {
                    semError(n->line, "type mismatch: cannot apply '%s' to '%s' operands",
                             opName(op), typeName(l != TYPE_FLOAT && l != TYPE_INT ? l : r));
                    n->type = TYPE_ERROR; return TYPE_ERROR;
                }
                n->type = (l == TYPE_FLOAT || r == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
                return n->type;
            }

            n->type = TYPE_ERROR; return TYPE_ERROR;
        }

        default:
            n->type = TYPE_ERROR; return TYPE_ERROR;
    }
}

/* ---- Walk a statement ---- */
static void checkStmt(Node *n) {
    if (!n) return;

    switch (n->kind) {
        case N_DECL: {
            /* Check the initializer first (it is evaluated in the current
               scope, before the new name becomes visible). */
            DataType initType = TYPE_NONE;
            if (n->left) initType = checkExpr(n->left);

            /* duplicate declaration check (same name, same scope) */
            Symbol *dup = lookupCurrentScope(n->sval);
            if (dup) {
                semError(n->line, "duplicate declaration of variable '%s'", n->sval, NULL);
            } else {
                int inited = (n->left != NULL);
                insertSymbol(n->sval, n->decl_type, n->line, inited);
                rememberName(n->sval);
            }

            /* initializer type must be assignable to the declared type */
            if (n->left && !assignable(n->decl_type, initType))
                semError(n->line,
                         "type mismatch: cannot assign '%s' to variable of type '%s'",
                         typeName(initType), typeName(n->decl_type));
            break;
        }

        case N_ASSIGN: {
            DataType rhs = checkExpr(n->left);
            Symbol *s = lookupSymbol(n->sval);
            if (!s) {
                if (wasEverDeclared(n->sval))
                    semError(n->line, "variable '%s' used outside its scope", n->sval, NULL);
                else
                    semError(n->line, "undeclared variable '%s'", n->sval, NULL);
            } else {
                if (!assignable(s->type, rhs))
                    semError(n->line,
                             "type mismatch: cannot assign '%s' to variable of type '%s'",
                             typeName(rhs), typeName(s->type));
                s->initialized = 1;   /* now has a value */
            }
            break;
        }

        case N_PRINT:
            checkExpr(n->left);
            break;

        case N_IF: {
            DataType c = checkExpr(n->left);
            if (c != TYPE_BOOL && c != TYPE_ERROR)
                semError(n->line, "if condition must be bool, got '%s'", typeName(c), NULL);
            checkStmt(n->right);           /* then-block (own scope)  */
            if (n->third) checkStmt(n->third);  /* else-block          */
            break;
        }

        case N_DOWHILE: {
            checkStmt(n->left);            /* body (own scope) */
            DataType c = checkExpr(n->right);
            if (c != TYPE_BOOL && c != TYPE_ERROR)
                semError(n->line, "while condition must be bool, got '%s'", typeName(c), NULL);
            break;
        }

        case N_BLOCK:
            enterScope();
            for (int i = 0; i < n->nstmts; i++) checkStmt(n->stmts[i]);
            exitScope();
            break;

        case N_PROGRAM:               /* top level = global scope (already open) */
            for (int i = 0; i < n->nstmts; i++) checkStmt(n->stmts[i]);
            break;

        default:
            break;
    }
}

int analyze(Node *root) {
    semantic_error_count = 0;
    declared_count = 0;
    initSymtab();                 /* opens global scope (level 0) */
    checkStmt(root);
    return semantic_error_count;
}
