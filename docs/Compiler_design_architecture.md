# Compiler Frontend — Project Build Guide

**Course:** Compiler Design Lab (CSE 3272)
**Institution:** Northern University of Bangladesh
**Team:** 6 Members

A lexer, parser, and Abstract Syntax Tree (AST) generator built from scratch in C — no external libraries, no dependencies.

---

## 1. What We're Building

A command-line program that takes a small source program as input and processes it through the first two phases of every real compiler:

1. **Lexical Analysis** - turns raw text into a stream of tokens
2. **Syntax Analysis** - turns tokens into a tree (AST) that reflects grammar and precedence

It also detects and reports syntax errors with line/column information instead of crashing.

### Example

**Input:**
```
x = 5 + y * (2 - 1)
```

**Token stream:**
```
IDENTIFIER(x) OPERATOR(=) NUMBER(5) OPERATOR(+)
IDENTIFIER(y) OPERATOR(*) LPAREN NUMBER(2) OPERATOR(-) NUMBER(1) RPAREN
```

**AST:**
```
Assign
├── Identifier: x
└── BinaryExpr (+)
    ├── Number: 5
    └── BinaryExpr (*)
        ├── Identifier: y
        └── BinaryExpr (-)
            ├── Number: 2
            └── Number: 1
```

**Error example:**
```
Input:  x = 5 +
Error (line 1, col 9): expected expression after '+'
```

---

## 2. Scope

### In Scope (Core Deliverable)
- Variable assignment: `x = <expr>`
- Arithmetic expressions: `+ - * /` with correct precedence and parentheses
- Identifiers and integer literals
- Syntax error detection with line/column reporting

### Stretch Goals (Only After Core Is Done)
- `if` / `while` control structures
- A tree-walking interpreter that evaluates the AST
- A simple symbol table for variables
- Basic semantic checks (e.g., undeclared variable)

### Explicitly Out of Scope
- Functions/procedures
- Type systems
- String/array data types
- Code generation / compilation to machine code

---

## 3. Grammar

```
statement   → IDENTIFIER '=' expr
expr        → term (('+' | '-') term)*
term        → factor (('*' | '/') factor)*
factor      → NUMBER | IDENTIFIER | '(' expr ')'
```

Each rule maps directly onto a parser function. Precedence is enforced by which function calls which: `parse_factor()` is called from inside `parse_term()`, and `parse_term()` from inside `parse_expr()`.

---

## 4. Project Structure

```
compiler-frontend/
├── src/
│   ├── main.c              # Entry point: reads input, runs lexer → parser → AST print
│   ├── lexer.c              # Tokenizer implementation
│   ├── lexer.h
│   ├── token.c              # Token type definitions and utilities
│   ├── token.h
│   ├── parser.c             # Recursive descent parser
│   ├── parser.h
│   ├── ast.c                # AST node definitions, tree construction, tree printing
│   ├── ast.h
│   ├── error.c              # Error reporting (line/column tracking, messages)
│   ├── error.h
│   ├── interpreter.c        # (Stretch) Evaluates the AST and prints results
│   └── interpreter.h
├── tests/
│   └── samples/
│       ├── valid_01.txt
│       ├── valid_02.txt
│       ├── broken_01.txt
│       └── broken_02.txt
├── Makefile
└── README.md
```

---

## 5. Core Data Structures

```c
// token.h
typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_KEYWORD,   // for if/while, stretch goal
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[64];
    int line;
    int column;
} Token;
```

```c
// ast.h
typedef enum {
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_BINARY_OP,
    NODE_ASSIGN
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char op;                  // '+', '-', '*', '/'
    int value;                 // for NODE_NUMBER
    char name[64];              // for NODE_IDENTIFIER / assignment target
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;
```

```c
// error.h
typedef struct {
    int line;
    int column;
    char message[256];
} CompilerError;

void report_error(int line, int column, const char *fmt, ...);
```

---

## 6. Module Responsibilities (Team of 6)

| # | Member | Module | What to Build |
|---|--------|--------|----------------|
| 1 | Md Kamruzzaman | Lexer Core & Integration | Character-by-character scanning loop, token generation, `main.c` pipeline wiring |
| 2 | Mubarak Hossain | Lexer Support | Keyword table, whitespace handling, line/column tracking |
| 3 | Abdullah Al Noman | Parser Core | `parse_expr()`, `parse_term()`, `parse_factor()` — precedence handling |
| 4 | Sadit Al Islam | Parser Extensions & AST | Assignment statement parsing, AST node construction |
| 5 | Soad Mahmud | AST Visualization | Tree-printing function used for the demo |
| 6 | Asraful Islam | Error Handling & QA | Error struct, message formatting, test suite, Makefile, README |

**Before writing code:** agree on the structs above and every function's exact signature as a team. This is what lets all 6 people work in parallel without blocking each other.

---

## 7. Function Signatures to Agree On (Day 1)

```c
// lexer.h
Token *tokenize(const char *source, int *token_count);
void free_tokens(Token *tokens);

// parser.h
ASTNode *parse(Token *tokens, int token_count);
void free_ast(ASTNode *node);

// ast.h
void print_ast(ASTNode *node, int depth);

// error.h
void report_error(int line, int column, const char *fmt, ...);
int had_error(void);
```

---

## 8. Build Plan (5 Weeks)

| Week | Milestone | What Gets Done |
|------|-----------|-----------------|
| 1 | Design Phase | Finalize grammar, structs, and function signatures. Set up shared repo and Makefile. |
| 2 | Lexer Development | Core scanning, token generation, line/column tracking, keyword recognition — built and unit-tested. |
| 3 | Parser & AST | Recursive-descent expression parsing, assignment statements, AST construction, tree-printing. |
| 4 | Integration & Error Handling | All modules merged into one pipeline; error reporting tested against broken sample programs. |
| 5 | Final Testing & Demo Prep | Full test suite run; stretch goals attempted if time allows; live demo rehearsed. |

---

## 9. Testing Strategy

Create paired test files in `tests/samples/`:

**Valid programs** (should parse cleanly):
```
x = 5
x = 5 + 3
x = (5 + 3) * 2
x = 5 + y * (2 - 1)
```

**Invalid programs** (should produce a clear error, not a crash):
```
x = 5 +          → expected expression after '+'
x = (5 + 2       → expected ')'
x = 5 # 2        → unexpected character '#'
= 5              → expected identifier
```

Run every sample through the binary and confirm output matches expectations before merging any module.

---

## 10. Building & Running

```bash
make
./compilerfrontend tests/samples/valid_01.txt
```

Expected output:
```
--- Tokens ---
IDENTIFIER(x) OPERATOR(=) NUMBER(5)

--- AST ---
Assign
├── Identifier: x
└── Number: 5
```

---

## 11. Minimal `Makefile` to Start From

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
SRC = src/main.c src/lexer.c src/token.c src/parser.c src/ast.c src/error.c
OBJ = $(SRC:.c=.o)
TARGET = compilerfrontend

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
```

---

## 12. Definition of Done (MVP)

- [ ] Lexer correctly tokenizes identifiers, numbers, operators, and parentheses
- [ ] Parser builds a correct AST respecting operator precedence
- [ ] Assignment statements parse correctly
- [ ] Errors report accurate line/column and a clear message
- [ ] Program never crashes/segfaults on malformed input
- [ ] Tree-printer produces readable output for the demo
- [ ] All test samples in `tests/samples/` pass

---

## 13. Stretch Goals (If Ahead of Schedule)

- [ ] `if` / `while` parsing added to the grammar
- [ ] Tree-walking interpreter that evaluates the AST and prints results
- [ ] Symbol table for variable storage during evaluation
- [ ] Undeclared-variable detection
