# =====================================================================
# Makefile  --  build recipe for the mini-language compiler
#
# Commands:
#   make            build the compiler  -> ./minilang   (default)
#   make all        same as `make` (this is the default target; IDEs call it)
#   make test       build, then run on a sample program
#   make lexer      build the Part A standalone token dumper -> ./lexer
#   make clean      delete all generated / compiled files
#
# Build order it knows:  bison -> parser.tab.c/.h ,  flex -> lex.yy.c ,
# then gcc links parser + lexer + ast into one program.
# =====================================================================

CC     = gcc
CFLAGS = -Wall -g -Wno-unused-function

# ---------- Default target ----------
# Named "all" so plain `make` AND IDEs (CLion runs `make all`) both work.
all: minilang

# ---------- Main build: the compiler ----------
minilang: parser.tab.c lex.yy.c ast.c ast.h symtab.c symtab.h semantic.c semantic.h
	$(CC) $(CFLAGS) -o minilang parser.tab.c lex.yy.c ast.c symtab.c semantic.c

# Bison turns the grammar into a C parser (parser.tab.c) and a header of
# token codes + the yylval union (parser.tab.h). -d makes the header.
parser.tab.c parser.tab.h: parser.y ast.h
	bison -d -o parser.tab.c parser.y

# Flex turns the lexer spec into a C scanner.
lex.yy.c: lexer.l
	flex -o lex.yy.c lexer.l

# ---------- Part A: standalone lexer (token dump) ----------
# Same lexer source, compiled with -DLEX_STANDALONE so it uses tokens.h
# and the lextest.c driver instead of the parser.
lexer: lex.yy.c lextest.c tokens.h
	$(CC) $(CFLAGS) -DLEX_STANDALONE -o lexer lex.yy.c lextest.c

.PHONY: all test clean
test: minilang
	./minilang tests/01_valid_arithmetic.ml

clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h *.o lexer minilang
