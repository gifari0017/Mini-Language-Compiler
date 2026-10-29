#ifndef AST_H
#define AST_H

/* =====================================================================
 * ast.h  --  Abstract Syntax Tree node definitions (Part B)
 *
 * The parser builds a TREE of these Node structs as it reads the program.
 * The tree captures the STRUCTURE of the code (which expression is inside
 * which statement, which statements are inside which block). Later phases
 * (symbol table, semantic analysis) walk this tree.
 *
 * Every node stores its source LINE number, required by the sheet for
 * error reporting.
 * ===================================================================== */

/* The data types our mini-language supports. TYPE_ERROR/NONE are internal
   helpers used during semantic analysis. */
typedef enum {
    TYPE_INT, TYPE_FLOAT, TYPE_STRING, TYPE_BOOL,
    TYPE_ERROR, TYPE_NONE
} DataType;

/* What KIND of node this is. One tag per language construct. */
typedef enum {
    N_PROGRAM,   /* whole program (list of statements)        */
    N_BLOCK,     /* { ... } a brace-delimited block            */
    N_DECL,      /* int x;   or   int x = expr;                */
    N_ASSIGN,    /* x = expr;                                  */
    N_IF,        /* if (cond) {..} else {..}                   */
    N_DOWHILE,   /* do {..} while (cond);                      */
    N_PRINT,     /* print expr;                                */
    N_BINOP,     /* expr OP expr   (+ - * / % == < && ...)     */
    N_UNOP,      /* ! expr   or   - expr                       */
    N_INT,       /* integer literal                            */
    N_FLOAT,     /* float literal                              */
    N_STRING,    /* string literal                             */
    N_BOOL,      /* true / false                               */
    N_VAR        /* use of a variable name                     */
} NodeKind;

/*
 * One flexible struct for ALL node kinds. Not every field is used by
 * every kind; each constructor fills only the fields that make sense.
 * (This "one struct fits all" style is the simplest to learn. A field
 * map is given with each constructor below.)
 */
typedef struct Node {
    NodeKind kind;
    int      line;        /* source line, for error messages            */
    DataType type;        /* RESULT type of an expression; set in Part D */

    /* leaf payloads */
    int      ival;        /* N_INT value; N_BOOL value (1=true,0=false)  */
    double   fval;        /* N_FLOAT value                               */
    char    *sval;        /* N_STRING text; N_VAR name; N_DECL/N_ASSIGN name */
    int      op;          /* operator token for N_BINOP / N_UNOP         */
    DataType decl_type;   /* declared type for N_DECL                    */

    /* child links (meaning depends on kind, see constructors) */
    struct Node *left;
    struct Node *right;
    struct Node *third;

    /* statement list, used by N_PROGRAM and N_BLOCK */
    struct Node **stmts;
    int           nstmts;
} Node;

/* ---------------- Constructors ----------------
 * Field map per kind:
 *   N_INT    : ival
 *   N_FLOAT  : fval
 *   N_STRING : sval
 *   N_BOOL   : ival (1/0)
 *   N_VAR    : sval = name
 *   N_BINOP  : op, left, right
 *   N_UNOP   : op, left = operand
 *   N_DECL   : decl_type, sval = name, left = init expr (or NULL)
 *   N_ASSIGN : sval = name, left = value expr
 *   N_IF     : left = cond, right = then-block, third = else-block (or NULL)
 *   N_DOWHILE: left = body-block, right = cond
 *   N_PRINT  : left = expr
 *   N_BLOCK  : stmts[]
 */
Node *newInt(int v, int line);
Node *newFloat(double v, int line);
Node *newString(char *s, int line);
Node *newBool(int v, int line);
Node *newVar(char *name, int line);
Node *newBinOp(int op, Node *l, Node *r, int line);
Node *newUnOp(int op, Node *operand, int line);
Node *newDecl(DataType t, char *name, Node *init, int line);
Node *newAssign(char *name, Node *expr, int line);
Node *newIf(Node *cond, Node *thenB, Node *elseB, int line);
Node *newDoWhile(Node *body, Node *cond, int line);
Node *newPrint(Node *expr, int line);
Node *newBlock(int line);
void  addStmt(Node *block, Node *stmt);   /* append stmt to a block/program */

/* ---------------- Helpers ---------------- */
const char *typeName(DataType t);   /* "int","float",...  for messages */
const char *opName(int op);         /* "+","==",...       for messages */
void printAST(Node *n, int indent); /* pretty-print the whole tree     */

#endif /* AST_H */
