/* =====================================================================
 * lextest.c  --  Standalone driver to TEST the lexer (Part A only)
 *
 * It repeatedly calls yylex(). Each call returns one token code (or 0 at
 * end of file). We print the line number, the token name, and the exact
 * text that was matched (the "lexeme"). This lets us SEE the token stream.
 *
 * In Part B this file is dropped; Bison's parser will call yylex() instead.
 * ===================================================================== */

#include <stdio.h>
#include "tokens.h"

/* Globals that Flex provides to us. */
extern int   yylex(void);       /* the scanner function Flex generated */
extern char *yytext;            /* text of the most recent match       */
extern int   yylineno;          /* current line number                 */
extern FILE *yyin;              /* input stream the lexer reads from    */
extern int   lex_error_count;   /* defined in lexer.l                  */

/* Map a token code to a human-readable name, for printing. */
static const char *token_name(int t) {
    switch (t) {
        case KW_INT:    return "KW_INT";
        case KW_FLOAT:  return "KW_FLOAT";
        case KW_STRING: return "KW_STRING";
        case KW_BOOL:   return "KW_BOOL";
        case KW_IF:     return "KW_IF";
        case KW_ELSE:   return "KW_ELSE";
        case KW_DO:     return "KW_DO";
        case KW_WHILE:  return "KW_WHILE";
        case KW_PRINT:  return "KW_PRINT";
        case INT_LIT:   return "INT_LIT";
        case FLOAT_LIT: return "FLOAT_LIT";
        case STRING_LIT:return "STRING_LIT";
        case BOOL_LIT:  return "BOOL_LIT";
        case ID:        return "ID";
        case PLUS:      return "PLUS";
        case MINUS:     return "MINUS";
        case MUL:       return "MUL";
        case DIV:       return "DIV";
        case MOD:       return "MOD";
        case ASSIGN:    return "ASSIGN";
        case EQ:        return "EQ";
        case NE:        return "NE";
        case LT:        return "LT";
        case LE:        return "LE";
        case GT:        return "GT";
        case GE:        return "GE";
        case AND:       return "AND";
        case OR:        return "OR";
        case NOT:       return "NOT";
        case LPAREN:    return "LPAREN";
        case RPAREN:    return "RPAREN";
        case LBRACE:    return "LBRACE";
        case RBRACE:    return "RBRACE";
        case SEMI:      return "SEMI";
        case COMMA:     return "COMMA";
        default:        return "UNKNOWN";
    }
}

int main(int argc, char **argv) {
    /* If a filename is given, read from it; otherwise read standard input. */
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Cannot open file: %s\n", argv[1]);
            return 1;
        }
    }

    printf("%-5s  %-12s  %s\n", "LINE", "TOKEN", "LEXEME");
    printf("-----  ------------  -------------------\n");

    int tok;
    int token_count = 0;
    while ((tok = yylex()) != 0) {        /* 0 means end of file */
        printf("%-5d  %-12s  %s\n", yylineno, token_name(tok), yytext);
        token_count++;
    }

    printf("\nScanned %d tokens, %d lexical error(s).\n",
           token_count, lex_error_count);
    return 0;
}
