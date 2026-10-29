/* =====================================================================
 * lexer.l  --  Flex specification for the mini-language lexer (Part A)
 *
 * A .l file has THREE sections, separated by lines containing only  %%
 *
 *   1. DEFINITIONS   (C code + named regex shortcuts)
 *   2. RULES         (regex  ->  action)   <-- the heart of the lexer
 *   3. USER CODE     (plain C, copied verbatim)
 *
 * Flex reads this file and generates a C function called yylex().
 * Each call to yylex() scans forward and returns ONE token code.
 * ===================================================================== */


/* ---------------- Section 1: DEFINITIONS ---------------- */
%{
    /* Anything between %{ and %} is raw C, copied to the top of the
       generated scanner. */
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #ifdef LEX_STANDALONE
        /* Part A mode: standalone token dump, driven by lextest.c */
        #include "tokens.h"
        #define SET_STR
        #define SET_INT
        #define SET_FLT
        #define SET_BOOL(x)
    #else
        /* Part B+ mode: driven by the Bison parser. parser.tab.h gives us
           the token codes AND the yylval union used to pass values up. */
        #include "parser.tab.h"
        /* Runs before every rule action: record the token's line into yylloc
           so the parser can report exact line numbers (@1.first_line). */
        #define YY_USER_ACTION  yylloc.first_line = yylineno; \
                                yylloc.last_line  = yylineno;
        /* Macros to hand the matched text/value to the parser via yylval. */
        #define SET_STR     yylval.str  = strdup(yytext);
        #define SET_INT     yylval.ival = atoi(yytext);
        #define SET_FLT     yylval.fval = atof(yytext);
        #define SET_BOOL(x) yylval.ival = (x);
    #endif

    /* Counts lexical errors so we can report a total at the end. */
    int lex_error_count = 0;
%}

/* %option lines configure Flex's behavior.
   noyywrap: single input file; don't require a yywrap() function.
   yylineno: Flex auto-maintains the global int yylineno, counting every
             '\n' inside any matched text, so line numbers "just work". */
%option noyywrap
%option yylineno

/* Named regex shortcuts. Use them below as {NAME}. Keeps rules readable. */
DIGIT   [0-9]
LETTER  [a-zA-Z_]
ID      {LETTER}({LETTER}|{DIGIT})*
INT     {DIGIT}+
FLOAT   {DIGIT}+"."{DIGIT}+


%%
 /* ---------------- Section 2: RULES ----------------
  *
  * Matching rules (how Flex picks a winner when input could match more
  * than one rule):
  *   1. LONGEST match wins.
  *   2. On a tie in length, the rule written FIRST wins.
  *
  * That is why keyword rules come BEFORE the identifier rule, and why
  * "==" is listed before "=".
  */

 /* ----- Comments: recognized, then skipped (no token returned) ----- */
"//"[^\n]*                      { /* single-line comment: ignore */ }
"/*"([^*]|\*+[^*/])*\*+"/"      { /* block comment: ignore. yylineno
                                     still counts any newlines inside. */ }

 /* ----- Whitespace: skipped. The \n is counted by yylineno. ----- */
[ \t\r\n]+                      { /* ignore */ }

 /* ----- Keywords (must appear before the ID rule) ----- */
"int"       { return KW_INT;    }
"float"     { return KW_FLOAT;  }
"string"    { return KW_STRING; }
"bool"      { return KW_BOOL;   }
"if"        { return KW_IF;     }
"else"      { return KW_ELSE;   }
"do"        { return KW_DO;     }
"while"     { return KW_WHILE;  }
"print"     { return KW_PRINT;  }
"true"      { SET_BOOL(1) return BOOL_LIT; }
"false"     { SET_BOOL(0) return BOOL_LIT; }

 /* ----- Multi-character operators (before their 1-char versions) ----- */
"=="        { return EQ;  }
"!="        { return NE;  }
"<="        { return LE;  }
">="        { return GE;  }
"&&"        { return AND; }
"||"        { return OR;  }

 /* ----- Single-character operators ----- */
"+"         { return PLUS;   }
"-"         { return MINUS;  }
"*"         { return MUL;    }
"/"         { return DIV;    }
"%"         { return MOD;    }
"="         { return ASSIGN; }
"<"         { return LT;     }
">"         { return GT;     }
"!"         { return NOT;    }

 /* ----- Punctuation ----- */
"("         { return LPAREN; }
")"         { return RPAREN; }
"{"         { return LBRACE; }
"}"         { return RBRACE; }
";"         { return SEMI;   }
","         { return COMMA;  }

 /* ----- Valid literals and identifiers -----
  * FLOAT listed before INT so "3.14" is taken whole, not as "3" "." "14".
  */
{FLOAT}                         { SET_FLT  return FLOAT_LIT;  }
{INT}                           { SET_INT  return INT_LIT;    }
\"[^\"\n]*\"                    { SET_STR  return STRING_LIT; }
{ID}                            { SET_STR  return ID;         }

 /* =================  LEXICAL ERROR RULES  =================
  * These are placed AFTER the valid rules. Because longest-match wins,
  * a malformed token (which is longer than the valid prefix) beats the
  * valid rule and triggers the error. On error we report and KEEP GOING
  * (we do not return a token), which is lexer-level panic recovery.
  */

 /* Malformed number: digits glued to letters/extra dots.
  * Catches  12abc   3.14xyz   (a number immediately followed by junk). */
{DIGIT}+("."{DIGIT}*)?[a-zA-Z_.][a-zA-Z0-9_.]*  {
        fprintf(stderr, "Lexical Error at line %d: malformed numeric literal '%s'\n",
                yylineno, yytext);
        lex_error_count++;
    }

 /* Unterminated string: opening quote, no closing quote before newline/EOF.
  * The valid-string rule above is longer when a closing quote exists,
  * so it wins in the good case; this rule only fires when none is found. */
\"[^\"\n]*                      {
        fprintf(stderr, "Lexical Error at line %d: unterminated string literal\n",
                yylineno);
        lex_error_count++;
    }

 /* Unterminated block comment: an opening slash-star with no close at EOF. */
"/*"([^*]|\*+[^*/])*            {
        fprintf(stderr, "Lexical Error at line %d: unterminated block comment\n",
                yylineno);
        lex_error_count++;
    }

 /* Catch-all: any single character we did not recognize is illegal. */
.                               {
        fprintf(stderr, "Lexical Error at line %d: illegal character '%s'\n",
                yylineno, yytext);
        lex_error_count++;
    }

%%
/* ---------------- Section 3: USER CODE ----------------
 * Left empty: the main() lives in lextest.c (Part A) and in the parser
 * (Part B). Keeping it out of here lets us reuse the same lexer in both. */
