# Compiler Frontend

A lexer, parser, and Abstract Syntax Tree (AST) generator for a small expression language, built from scratch in C — no external libraries, no dependencies.

Full design spec: [docs/Compiler_design_architecture.md](docs/Compiler_design_architecture.md)
Implementation plan: [docs/MILESTONES.md](docs/MILESTONES.md)

## What it does

Reads a source file, tokenizes it, parses it into an AST respecting operator precedence, and prints both. Malformed input produces a clear error with line/column instead of crashing.

```
statement   → assign_stmt | if_stmt | while_stmt
assign_stmt → IDENTIFIER '=' expr
if_stmt     → 'if' '(' condition ')' block
while_stmt  → 'while' '(' condition ')' block
block       → '{' statement* '}'
condition   → expr (('<' | '>' | '<=' | '>=' | '==' | '!=') expr)?
expr        → term (('+' | '-') term)*
term        → factor (('*' | '/') factor)*
factor      → NUMBER | IDENTIFIER | '(' expr ')'
```

The top level accepts exactly one statement (which may itself be an `if`/`while` whose block holds many) — this keeps the original core grammar's guarantees intact rather than introducing multi-statement programs.

Pass `--eval` to additionally run the tree-walking interpreter, which executes the AST, maintains a variable symbol table, and prints each assignment as it happens. Referencing a variable before it's assigned is a runtime error (reported with line/column, no crash).

## Prerequisites

Requires a C compiler (`gcc`) and `make`. If you don't already have them:

**Ubuntu / Debian**
```bash
sudo apt update
sudo apt install build-essential
```

**macOS**
```bash
xcode-select --install
```
(Installs Apple's Command Line Tools — `clang` aliased as `gcc`, plus `make`.)

**Windows**

No compiler ships with Windows. Pick one:

- Git Bash / MSYS2 shell available:
  ```bash
  pacman -S mingw-w64-x86_64-gcc make
  ```
- PowerShell only, via [winget](https://learn.microsoft.com/windows/package-manager/winget/):
  1. Install the toolchain:
     ```powershell
     winget install -e --id BrechtSanders.WinLibs.POSIX.UCRT
     ```
  2. Find where it installed (typically under `C:\Program Files\WinLibs...\mingw64\bin`), then add that folder to PATH for the current session:
     ```powershell
     $env:PATH += ";C:\Program Files\WinLibs-x86_64-...\mingw64\bin"
     ```
     For a permanent change, add it via System Properties → Environment Variables, then restart PowerShell.
  3. Verify:
     ```powershell
     gcc --version
     mingw32-make --version
     ```
- PowerShell only, via [Chocolatey](https://chocolatey.org/):
  ```powershell
  choco install mingw -y
  ```

Verify with `gcc --version` and `make --version` (or `mingw32-make --version` on Windows).

> **Note:** On Windows, `make` is often not on PATH — use `mingw32-make` instead everywhere `make` is referenced below. `make test` additionally needs a bash shell (Git Bash, MSYS2, or WSL) since it runs a `.sh` script; it will not run from plain PowerShell.

## Building

```bash
make
```

On Windows without MSYS2's `make`, use `mingw32-make` instead.

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

### Interpreter (`--eval`)

```
$ ./compilerfrontend tests/samples/eval_03.txt --eval
...
--- Evaluation ---
x = 0
x = 1
x = 2
x = 3
```

## Testing

```bash
make test
```

Runs every file in `tests/samples/*.txt` against its paired `*.expected` transcript and reports pass/fail. All 16 samples must pass.

On Windows, `make test` needs a bash shell (Git Bash, MSYS2, or WSL) since it invokes `tests/run_tests.sh` — it won't run from plain PowerShell.

To add a new sample: drop `name.txt` in `tests/samples/`, run the binary once to capture its output, save that as `tests/samples/name.expected`, then re-run `make test`. Files named `valid_*`/`eval_*` are expected to exit 0; files named `broken_*` are expected to exit 1; any name containing `eval` is run with `--eval`.

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
│   └── interpreter.c/h # Tree-walking evaluator + symbol table (--eval)
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
