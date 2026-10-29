/* =====================================================================
 * ast.c  --  AST node constructors + helpers (Part B)
 * ===================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"   /* for the operator token codes (PLUS, EQ, ...) */

/* Allocate a blank node of a given kind. Shared by all constructors. */
static Node *newNode(NodeKind kind, int line) {
    Node *n = (Node *)calloc(1, sizeof(Node));  /* calloc zeroes everything */
    n->kind = kind;
    n->line = line;
    n->type = TYPE_NONE;
    return n;
}

Node *newInt(int v, int line)     { Node *n=newNode(N_INT,line);   n->ival=v; return n; }
Node *newFloat(double v,int line) { Node *n=newNode(N_FLOAT,line); n->fval=v; return n; }
Node *newString(char *s,int line) { Node *n=newNode(N_STRING,line);n->sval=s; return n; }
Node *newBool(int v,int line)     { Node *n=newNode(N_BOOL,line);  n->ival=v; return n; }
Node *newVar(char *name,int line) { Node *n=newNode(N_VAR,line);   n->sval=name; return n; }

Node *newBinOp(int op, Node *l, Node *r, int line) {
    Node *n=newNode(N_BINOP,line); n->op=op; n->left=l; n->right=r; return n;
}
Node *newUnOp(int op, Node *operand, int line) {
    Node *n=newNode(N_UNOP,line); n->op=op; n->left=operand; return n;
}
Node *newDecl(DataType t, char *name, Node *init, int line) {
    Node *n=newNode(N_DECL,line); n->decl_type=t; n->sval=name; n->left=init; return n;
}
Node *newAssign(char *name, Node *expr, int line) {
    Node *n=newNode(N_ASSIGN,line); n->sval=name; n->left=expr; return n;
}
Node *newIf(Node *cond, Node *thenB, Node *elseB, int line) {
    Node *n=newNode(N_IF,line); n->left=cond; n->right=thenB; n->third=elseB; return n;
}
Node *newDoWhile(Node *body, Node *cond, int line) {
    Node *n=newNode(N_DOWHILE,line); n->left=body; n->right=cond; return n;
}
Node *newPrint(Node *expr, int line) {
    Node *n=newNode(N_PRINT,line); n->left=expr; return n;
}
Node *newBlock(int line) { return newNode(N_BLOCK,line); }

/* Append a statement to a block/program's growing stmts[] array. */
void addStmt(Node *block, Node *stmt) {
    if (!block || !stmt) return;
    block->stmts = (Node **)realloc(block->stmts,
                                    sizeof(Node *) * (block->nstmts + 1));
    block->stmts[block->nstmts++] = stmt;
}

const char *typeName(DataType t) {
    switch (t) {
        case TYPE_INT:    return "int";
        case TYPE_FLOAT:  return "float";
        case TYPE_STRING: return "string";
        case TYPE_BOOL:   return "bool";
        case TYPE_ERROR:  return "error";
        default:          return "none";
    }
}

/* Map an operator token code to its printable symbol. */
const char *opName(int op) {
    switch (op) {
        case PLUS: return "+";   case MINUS: return "-";
        case MUL:  return "*";   case DIV:   return "/";  case MOD: return "%";
        case EQ:   return "==";  case NE:    return "!=";
        case LT:   return "<";   case LE:    return "<=";
        case GT:   return ">";   case GE:    return ">=";
        case AND:  return "&&";  case OR:    return "||";  case NOT: return "!";
        default:   return "?";
    }
}

/* ---- Pretty-printer: shows the tree with indentation + line numbers ---- */
static void indentBy(int n) { for (int i=0;i<n;i++) printf("  "); }

void printAST(Node *n, int indent) {
    if (!n) return;
    indentBy(indent);
    switch (n->kind) {
        case N_PROGRAM:
            printf("Program\n");
            for (int i=0;i<n->nstmts;i++) printAST(n->stmts[i], indent+1);
            break;
        case N_BLOCK:
            printf("Block (line %d)\n", n->line);
            for (int i=0;i<n->nstmts;i++) printAST(n->stmts[i], indent+1);
            break;
        case N_DECL:
            printf("Decl: %s %s (line %d)\n", typeName(n->decl_type), n->sval, n->line);
            if (n->left) { indentBy(indent+1); printf("init:\n"); printAST(n->left, indent+2); }
            break;
        case N_ASSIGN:
            printf("Assign: %s (line %d)\n", n->sval, n->line);
            printAST(n->left, indent+1);
            break;
        case N_IF:
            printf("If (line %d)\n", n->line);
            indentBy(indent+1); printf("cond:\n");  printAST(n->left,  indent+2);
            indentBy(indent+1); printf("then:\n");  printAST(n->right, indent+2);
            if (n->third) { indentBy(indent+1); printf("else:\n"); printAST(n->third, indent+2); }
            break;
        case N_DOWHILE:
            printf("DoWhile (line %d)\n", n->line);
            indentBy(indent+1); printf("body:\n"); printAST(n->left,  indent+2);
            indentBy(indent+1); printf("cond:\n"); printAST(n->right, indent+2);
            break;
        case N_PRINT:
            printf("Print (line %d)\n", n->line);
            printAST(n->left, indent+1);
            break;
        case N_BINOP:
            printf("BinaryOp '%s' (line %d)\n", opName(n->op), n->line);
            printAST(n->left,  indent+1);
            printAST(n->right, indent+1);
            break;
        case N_UNOP:
            printf("UnaryOp '%s' (line %d)\n", opName(n->op), n->line);
            printAST(n->left, indent+1);
            break;
        case N_INT:    printf("Int %d (line %d)\n",    n->ival, n->line); break;
        case N_FLOAT:  printf("Float %g (line %d)\n",  n->fval, n->line); break;
        case N_STRING: printf("String %s (line %d)\n", n->sval, n->line); break;
        case N_BOOL:   printf("Bool %s (line %d)\n", n->ival?"true":"false", n->line); break;
        case N_VAR:    printf("Var %s (line %d)\n",    n->sval, n->line); break;
        default:       printf("Unknown node\n");
    }
}
