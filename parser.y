/* =====================================================================
 * parser.y  --  Bison grammar + AST construction (Part B)
 *
 * Bison reads this and generates a parser function yyparse(). The parser
 * repeatedly asks the lexer (yylex) for tokens and checks they fit the
 * grammar rules below. In each rule's action { ... } we BUILD an AST node,
 * so by the time the whole program is parsed we have the full tree in `root`.
 *
 * Error recovery: PANIC MODE. On a syntax error, the special `error` token
 * makes the parser discard tokens until it reaches a ';' , then it resumes.
 * This lets us report MANY syntax errors in one run instead of stopping at
 * the first.
 * ===================================================================== */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantic.h"
#include "symtab.h"

extern int   yylex(void);
extern int   yylineno;
extern FILE *yyin;
extern int   lex_error_count;     /* defined in lexer.l */

void yyerror(const char *s);

Node *root = NULL;                 /* root of the finished AST */
int   syntax_error_count = 0;
%}

/* Track line/column for every symbol, so @1.first_line gives exact lines. */
%locations
/* Detailed messages ("unexpected X, expecting Y"); LAC makes the
   "expecting" set accurate. We reshape the text in yyerror() below. */
%define parse.error verbose
%define parse.lac full

/* The possible types a grammar symbol's value can hold. */
%union {
    struct Node *node;
    char        *str;
    int          ival;
    double       fval;
}

/* ---- Tokens that carry a value (from the lexer via yylval) ---- */
%token <ival> INT_LIT
%token <fval> FLOAT_LIT
%token <str>  STRING_LIT
%token <ival> BOOL_LIT
%token <str>  ID

/* ---- Keyword tokens. The string alias (e.g. "if") is what shows up in
        error messages, making them human-readable. ---- */
%token KW_INT "int"  KW_FLOAT "float"  KW_STRING "string"  KW_BOOL "bool"
%token KW_IF "if"  KW_ELSE "else"  KW_DO "do"  KW_WHILE "while"  KW_PRINT "print"

/* ---- Operators and punctuation ---- */
%token PLUS "+"  MINUS "-"  MUL "*"  DIV "/"  MOD "%"
%token ASSIGN "="  EQ "=="  NE "!="  LT "<"  LE "<="  GT ">"  GE ">="
%token AND "&&"  OR "||"  NOT "!"
%token LPAREN "("  RPAREN ")"  LBRACE "{"  RBRACE "}"  SEMI ";"  COMMA ","

/* ---- Types of the nonterminals (what AST value each produces) ---- */
%type <node> stmt_list stmt decl_stmt assign_stmt if_stmt dowhile_stmt print_stmt block expr
%type <ival> type

/* ---- Operator precedence, lowest first. Resolves expression ambiguity
        (e.g. a + b * c parses as a + (b*c)). ---- */
%left OR
%left AND
%left EQ NE
%left LT LE GT GE
%left PLUS MINUS
%left MUL DIV MOD
%right NOT UMINUS          /* unary ! and unary - : highest, right-assoc */

%start program

%%

/* ---------------- Grammar rules ---------------- */

program
    : stmt_list                 { $1->kind = N_PROGRAM; root = $1; }
    ;

/* A list of statements. Left-recursive: grows one statement at a time.
   Starts as an empty block node, then each statement is appended. */
stmt_list
    : /* empty */               { $$ = newBlock(yylineno); }
    | stmt_list stmt            { if ($2) addStmt($1, $2); $$ = $1; }
    ;

stmt
    : decl_stmt
    | assign_stmt
    | if_stmt
    | dowhile_stmt
    | print_stmt
    | block
    | error SEMI                { $$ = NULL; yyerrok; }   /* PANIC-MODE recovery */
    ;

/* A type keyword yields a DataType value (carried as int). */
type
    : "int"     { $$ = TYPE_INT;    }
    | "float"   { $$ = TYPE_FLOAT;  }
    | "string"  { $$ = TYPE_STRING; }
    | "bool"    { $$ = TYPE_BOOL;   }
    ;

decl_stmt
    : type ID SEMI              { $$ = newDecl((DataType)$1, $2, NULL, @2.first_line); }
    | type ID ASSIGN expr SEMI  { $$ = newDecl((DataType)$1, $2, $4,   @2.first_line); }
    ;

assign_stmt
    : ID ASSIGN expr SEMI       { $$ = newAssign($1, $3, @1.first_line); }
    ;

print_stmt
    : "print" expr SEMI         { $$ = newPrint($2, @1.first_line); }
    ;

/* Bodies are blocks { } . Requiring braces removes the classic
   "dangling else" ambiguity and keeps the grammar simple. */
if_stmt
    : "if" "(" expr ")" block                 { $$ = newIf($3, $5, NULL, @1.first_line); }
    | "if" "(" expr ")" block "else" block     { $$ = newIf($3, $5, $7,   @1.first_line); }
    ;

dowhile_stmt
    : "do" block "while" "(" expr ")" SEMI     { $$ = newDoWhile($2, $5, @1.first_line); }
    ;

block
    : "{" stmt_list "}"         { $2->line = @1.first_line; $$ = $2; }
    ;

expr
    : expr "+"  expr            { $$ = newBinOp(PLUS,  $1, $3, @2.first_line); }
    | expr "-"  expr            { $$ = newBinOp(MINUS, $1, $3, @2.first_line); }
    | expr "*"  expr            { $$ = newBinOp(MUL,   $1, $3, @2.first_line); }
    | expr "/"  expr            { $$ = newBinOp(DIV,   $1, $3, @2.first_line); }
    | expr "%"  expr            { $$ = newBinOp(MOD,   $1, $3, @2.first_line); }
    | expr "==" expr            { $$ = newBinOp(EQ,    $1, $3, @2.first_line); }
    | expr "!=" expr            { $$ = newBinOp(NE,    $1, $3, @2.first_line); }
    | expr "<"  expr            { $$ = newBinOp(LT,    $1, $3, @2.first_line); }
    | expr "<=" expr            { $$ = newBinOp(LE,    $1, $3, @2.first_line); }
    | expr ">"  expr            { $$ = newBinOp(GT,    $1, $3, @2.first_line); }
    | expr ">=" expr            { $$ = newBinOp(GE,    $1, $3, @2.first_line); }
    | expr "&&" expr            { $$ = newBinOp(AND,   $1, $3, @2.first_line); }
    | expr "||" expr            { $$ = newBinOp(OR,    $1, $3, @2.first_line); }
    | "!" expr                  { $$ = newUnOp(NOT,   $2, @1.first_line); }
    | "-" expr %prec UMINUS     { $$ = newUnOp(MINUS, $2, @1.first_line); }
    | "(" expr ")"              { $$ = $2; }
    | INT_LIT                   { $$ = newInt($1,    @1.first_line); }
    | FLOAT_LIT                 { $$ = newFloat($1,  @1.first_line); }
    | STRING_LIT                { $$ = newString($1, @1.first_line); }
    | BOOL_LIT                  { $$ = newBool($1,   @1.first_line); }
    | ID                        { $$ = newVar($1,    @1.first_line); }
    ;

%%

/* ---------------- Support code ---------------- */

/* Bison calls yyerror() on each syntax error. In verbose mode `s` looks
   like:  "syntax error, unexpected \"int\", expecting \";\""
   We pull out the "found" and "expecting" parts and reprint in the
   sheet's format:  Syntax Error at line N: expected Y but found X  */
void yyerror(const char *s) {
    syntax_error_count++;

    const char *found = NULL, *expecting = NULL;
    static char fbuf[160];

    const char *u = strstr(s, "unexpected ");
    const char *e = strstr(s, ", expecting ");
    if (u) {
        u += strlen("unexpected ");
        if (e) {
            int len = (int)(e - u);
            if (len > 159) len = 159;
            strncpy(fbuf, u, len); fbuf[len] = '\0';
            found = fbuf;
            expecting = e + strlen(", expecting ");
        } else {
            found = u;                 /* no "expecting" part present */
        }
    }

    fprintf(stderr, "Syntax Error at line %d: ", yylloc.first_line);
    if (expecting && found)
        fprintf(stderr, "expected %s but found %s\n", expecting, found);
    else if (found)
        fprintf(stderr, "unexpected %s\n", found);
    else
        fprintf(stderr, "%s\n", s);    /* fallback: raw message */
}

int main(int argc, char **argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) { fprintf(stderr, "Cannot open file: %s\n", argv[1]); return 1; }
    }

    yyparse();

    int front_errors = lex_error_count + syntax_error_count;

    /* Only run semantic analysis if the front-end produced a complete tree.
       With syntax errors the AST may be partial, so we skip it then. */
    if (front_errors == 0) {
        printf("Parse successful. Abstract Syntax Tree:\n\n");
        printAST(root, 0);

        int sem = analyze(root);     /* Parts C + D: build symtab, check */
        printSymtab();

        if (sem == 0)
            printf("\nNo semantic errors. Compilation front-end succeeded.\n");
        else
            printf("\nFinished with %d semantic error(s).\n", sem);
        return sem ? 1 : 0;
    } else {
        printf("\nFinished with %d lexical error(s) and %d syntax error(s).\n",
               lex_error_count, syntax_error_count);
        return 1;
    }
}
