#ifndef TOKENS_H
#define TOKENS_H

/*
 * Token codes for the mini-language.
 *
 * These are just integer IDs. The lexer returns one of these every time
 * it recognizes a "word" (token) in the source code.
 *
 * We start numbering at 256 on purpose: values 0-255 are reserved for
 * single raw characters and for the special EOF value (0), so starting
 * at 256 guarantees our tokens never collide with those.
 *
 * NOTE: This file is only used in Part A (standalone lexer test).
 * In Part B, Bison generates these same names in "parser.tab.h" and we
 * include that instead. The names stay identical, so the lexer code does
 * not change when we switch.
 */

enum {
    /* ---- Keywords ---- */
    KW_INT = 256,   /* int    */
    KW_FLOAT,       /* float  */
    KW_STRING,      /* string */
    KW_BOOL,        /* bool   */
    KW_IF,          /* if     */
    KW_ELSE,        /* else   */
    KW_DO,          /* do     */
    KW_WHILE,       /* while  */
    KW_PRINT,       /* print  */

    /* ---- Literals (actual values in the source) ---- */
    INT_LIT,        /* 42        */
    FLOAT_LIT,      /* 3.14      */
    STRING_LIT,     /* "hello"   */
    BOOL_LIT,       /* true / false */

    /* ---- Identifier (a variable name) ---- */
    ID,             /* x, count, myVar */

    /* ---- Operators ---- */
    PLUS,           /* +  */
    MINUS,          /* -  */
    MUL,            /* *  */
    DIV,            /* /  */
    MOD,            /* %  */
    ASSIGN,         /* =  */
    EQ,             /* == */
    NE,             /* != */
    LT,             /* <  */
    LE,             /* <= */
    GT,             /* >  */
    GE,             /* >= */
    AND,            /* && */
    OR,             /* || */
    NOT,            /* !  */

    /* ---- Punctuation ---- */
    LPAREN,         /* ( */
    RPAREN,         /* ) */
    LBRACE,         /* { */
    RBRACE,         /* } */
    SEMI,           /* ; */
    COMMA           /* , */
};

#endif /* TOKENS_H */
