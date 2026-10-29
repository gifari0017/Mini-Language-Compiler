# Mini-Language Compiler

[![build](https://github.com/gifari0017/Mini-Language-Compiler/actions/workflows/ci.yml/badge.svg)](https://github.com/gifari0017/Mini-Language-Compiler/actions/workflows/ci.yml)
![language](https://img.shields.io/badge/language-C-00599C)
![flex](https://img.shields.io/badge/lexer-Flex-brightgreen)
![bison](https://img.shields.io/badge/parser-GNU%20Bison-orange)
![license](https://img.shields.io/badge/license-MIT-blue)

The front-end of a compiler for a small statically-typed language, built for
**CSE 2216 (Compiler Laboratory), Lab 07**. It takes a source program and runs
it through the four classic front-end phases, reporting errors with line
numbers at every stage:

1. **Lexical analysis** — group characters into tokens *(Flex)*
2. **Syntax analysis + AST construction** — validate the grammar and build a
   real Abstract Syntax Tree *(GNU Bison)*
3. **Symbol table** — record every identifier with type, scope, line, and
   initialization status
4. **Semantic analysis** — type checking and scope/declaration rules

```
source.ml → [ LEXER ] → tokens → [ PARSER ] → AST → [ SYMBOL TABLE ] → [ SEMANTIC ANALYSIS ] → verdict
```

## Quick start

Requires `flex`, `bison`, `gcc`, and `make` (on Ubuntu/WSL:
`sudo apt install flex bison build-essential`).

```bash
make                                  # build -> ./minilang
./minilang tests/01_valid_arithmetic.ml   # run on one program
./run_tests.sh                        # run on all 12 test programs
make clean                            # remove generated/compiled files
```

## The language

```c
int    count = 10;          // int, float, string, bool
float  pi    = 3.14;
string name  = "keya";
bool   ok    = true;

do {
    count = count + 1;
} while (count < 20);

if (count >= 20 && ok) {
    print "done";
} else {
    print count;
}
```

- **Types:** `int`, `float`, `string`, `bool`
- **Operators:** `+ - * / %`, `== != < <= > >=`, `&& || !`
- **Control flow:** `if / else`, `do / while` (bodies must use `{ }`)
- **Statements:** declaration, assignment, `print`, blocks
- **Comments:** `//` single-line and `/* ... */` block

## What it catches

| Phase | Errors detected |
|-------|-----------------|
| **Lexical** | illegal characters, malformed numbers (`12abc`, `12.3.4`), unterminated strings |
| **Syntax** | missing/unexpected tokens, unmatched `()`/`{}`, malformed statements (panic-mode recovery reports several per run) |
| **Semantic** | undeclared variables, duplicate declarations, type mismatches in expressions, assignment mismatches, scope violations, uninitialized use |

All phases report through one format: `<Phase> Error at line N: <message>`.

## Example output

Running on a valid program prints the AST and the symbol table:

```
$ ./minilang tests/01_valid_arithmetic.ml
Parse successful. Abstract Syntax Tree:

Program
  Decl: int a (line 2)
    init:
      BinaryOp '+' (line 2)
        Int 2 (line 2)
        BinaryOp '*' (line 2)      <-- a = 2 + 3 * 4 parsed with correct precedence
          Int 3 (line 2)
          Int 4 (line 2)
  ...
```

Running on a program with mistakes reports each one with its line, then dumps
the symbol table:

```
$ ./minilang tests/12_sem_scope_uninit.ml
Semantic Error at line 3: variable 'total' used before initialization
Semantic Error at line 9: variable 'temp' used outside its scope

==================== SYMBOL TABLE ====================
NAME          TYPE      SCOPE   DECL_LINE  INITIALIZED
-----------------------------------------------------
total         int       0       2          no
next          int       0       3          yes
temp          int       1       6          yes
=====================================================

Finished with 2 semantic error(s).
```

## Project layout

```
minilang/
├── Makefile              build recipe (make / make test / make clean)
├── tokens.h              token codes (Part A standalone)
├── lexer.l               Flex lexer specification            (Part A)
├── lextest.c             standalone token-dump driver        (Part A)
├── ast.h  ast.c          AST node types + constructors       (Part B)
├── parser.y              Bison grammar + AST construction     (Part B)
├── symtab.h  symtab.c    symbol table (scope stack)          (Part C)
├── semantic.h semantic.c semantic analysis + type checking   (Part D)
├── tests/                12 test programs (valid + each error class)
├── run_tests.sh          runs the compiler on every test
└── report.docx / .pdf    full project report
```

## Error recovery

The parser uses **panic-mode recovery**: on a syntax error it discards tokens
up to the next `;` and resumes, so one run can surface several syntax errors
instead of stopping at the first. Semantic analysis runs only when parsing
finishes cleanly, so diagnostics are reported against a complete AST.

## Team

| Member | Part |
|--------|------|
| Shahriar Sourov | A — Lexical analyzer |
| Nukhyai U Marma | B — Parser and AST |
| Saad Bin Sultan | C — Symbol table |
| Abdus Salam Gifari | D — Semantic analysis |

## License

Released under the [MIT License](LICENSE).
