# Compiler Frontend

A lexer, parser, and Abstract Syntax Tree (AST) generator for a small expression language, built from scratch in C — no external libraries, no dependencies.

Full design spec: [docs/Compiler_design_architecture.md](docs/Compiler_design_architecture.md)
Implementation plan: [docs/MILESTONES.md](docs/MILESTONES.md)

## What it does

Reads a source file, tokenizes it, parses it into an AST respecting operator precedence, and prints both. Malformed input produces a clear error with line/column instead of crashing.

```
statement   → IDENTIFIER '=' expr
expr        → term (('+' | '-') term)*
term        → factor (('*' | '/') factor)*
factor      → NUMBER | IDENTIFIER | '(' expr ')'
```

## Building

Requires `gcc` and `make` (or `mingw32-make` on Windows).

```bash
make
```

Produces `compilerfrontend` (or `compilerfrontend.exe` on Windows) with zero warnings under `-Wall -Wextra -std=c99`.

## Running

```bash
./compilerfrontend tests/samples/valid_04.txt
```

```
--- Tokens ---
IDENTIFIER(x) OPERATOR(=) NUMBER(5) OPERATOR(+) IDENTIFIER(y) OPERATOR(*) LPAREN NUMBER(2) OPERATOR(-) NUMBER(1) RPAREN

--- AST ---
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

On a syntax error, the program prints tokens successfully lexed (if any), reports the error with line/column to stderr, and exits with a non-zero status:

```
$ ./compilerfrontend tests/samples/broken_01.txt
--- Tokens ---
IDENTIFIER(x) OPERATOR(=) NUMBER(5) OPERATOR(+)
Error (line 1, col 8): expected expression after '+'
```

## Testing

```bash
make test
```

Runs every file in `tests/samples/*.txt` against its paired `*.expected` transcript and reports pass/fail. All 8 samples (4 valid, 4 broken) must pass.

To add a new sample: drop `name.txt` in `tests/samples/`, run the binary once to capture its output, save that as `tests/samples/name.expected`, then re-run `make test`. Files named `valid_*` are expected to exit 0; files named `broken_*` are expected to exit 1.

## Project structure

```
compiler-frontend/
├── src/
│   ├── main.c         # Entry point: reads input, runs lexer → parser → AST print
│   ├── lexer.c/h       # Tokenizer
│   ├── token.c/h       # Token type definitions and utilities
│   ├── parser.c/h      # Recursive descent parser
│   ├── ast.c/h         # AST node definitions, tree construction, tree printing
│   ├── error.c/h       # Error reporting (line/column tracking, messages)
│   └── interpreter.c/h # (stretch) evaluates the AST
├── tests/
│   ├── run_tests.sh
│   └── samples/         # *.txt inputs paired with *.expected transcripts
├── docs/
│   ├── Compiler_design_architecture.md
│   └── MILESTONES.md
└── Makefile
```

## Cleaning

```bash
make clean
```
