# Implementation Milestones — Compiler Frontend

Source spec: [docs/Compiler_design_architecture.md](docs/Compiler_design_architecture.md)

Each milestone is implemented, built, and tested before moving to the next. Core milestones (M1–M5) deliver the MVP defined in the spec's Definition of Done. M6–M7 are stretch goals, attempted only after M5 is fully green.

---

## M1 — Project Scaffolding & Core Types

**Goal:** A buildable skeleton with the shared data structures every other module depends on, so parallel work never blocks on undefined types.

**Files added:**
- `Makefile`
- `.gitignore` (build artifacts: `*.o`, `compilerfrontend`, `compilerfrontend.exe`)
- `src/token.h` — `TokenType` enum, `Token` struct
- `src/token.c` — token utility (e.g. `token_type_name()` for debug printing)
- `src/error.h` — `CompilerError` struct, `report_error()`, `had_error()` declarations
- `src/error.c` — implementation (stores/prints errors, tracks error flag)
- `src/main.c` — stub entry point (reads filename arg, reads file into buffer, prints "OK")

**Acceptance criteria:**
- [ ] `make` builds cleanly with `-Wall -Wextra -std=c99` and zero warnings
- [ ] `./compilerfrontend tests/samples/valid_01.txt` runs without crashing (even if it does nothing yet)
- [ ] `token.h` matches the struct shape in the spec (§5)
- [ ] `report_error(line, col, fmt, ...)` formats a message and sets an internal error flag; `had_error()` reflects it
- [ ] `make clean` removes all build artifacts

---

## M2 — Lexer

**Goal:** Full tokenization of the language: identifiers, integer literals, operators, parentheses, keywords (for stretch), whitespace/line/column tracking, EOF.

**Files added/changed:**
- `src/lexer.h` — `tokenize()`, `free_tokens()` declarations
- `src/lexer.c` — scanning loop, keyword table (`if`/`while` recognized but unused until M6), line/column tracking
- `src/main.c` — wire up `tokenize()`, print token stream (`--- Tokens ---` block from spec §1)
- `tests/samples/valid_01.txt` … `valid_04.txt` — the four valid programs from spec §9

**Acceptance criteria:**
- [ ] `x = 5 + y * (2 - 1)` tokenizes to exactly the stream shown in spec §1
- [ ] Line and column numbers are correct for every token (1-indexed)
- [ ] Whitespace/newlines are skipped and don't produce tokens but do advance line/col correctly
- [ ] Unrecognized characters (e.g. `#`) produce a lexer-level error via `report_error()` instead of crashing
- [ ] `free_tokens()` frees the token array with no leaks (spot-checked; full leak audit in M5)
- [ ] All 4 valid sample files tokenize without error

---

## M3 — Parser & AST

**Goal:** Recursive-descent parser implementing the grammar in spec §3, building an AST, and printing it as a tree.

**Files added/changed:**
- `src/ast.h` — `NodeType` enum, `ASTNode` struct, `print_ast()`, `free_ast()` declarations
- `src/ast.c` — node constructors, `print_ast()` (tree-drawing with `├──`/`└──`), `free_ast()`
- `src/parser.h` — `parse()` declaration
- `src/parser.c` — `parse_statement()`, `parse_expr()`, `parse_term()`, `parse_factor()`
- `src/main.c` — wire up `parse()` + `print_ast()` after tokenization (`--- AST ---` block)

**Acceptance criteria:**
- [ ] `x = 5 + y * (2 - 1)` produces the exact AST shape shown in spec §1 (precedence and associativity correct)
- [ ] `parse_factor` → `parse_term` → `parse_expr` call chain enforces `* /` binding tighter than `+ -`
- [ ] Parenthesized sub-expressions override precedence correctly
- [ ] `free_ast()` recursively frees every node with no leaks (spot-checked)
- [ ] All 4 valid sample files produce a correct AST end-to-end (tokens → parse → print)

---

## M4 — Integration & Error Handling

**Goal:** One robust pipeline (`main.c`) that never crashes on malformed input and reports precise, useful syntax errors.

**Files added/changed:**
- `src/main.c` — finalize pipeline: read file → tokenize → check `had_error()` → parse → check `had_error()` → print AST; nonzero exit code on error
- `src/parser.c` — error productions for every grammar rule (expected identifier, expected expression, expected `)`, unexpected token/EOF)
- `src/error.c` — refine message formatting to match spec §9 examples exactly
- `tests/samples/broken_01.txt` … `broken_04.txt` — the four invalid programs from spec §9

**Acceptance criteria:**
- [ ] `x = 5 +` → `Error (line 1, col 9): expected expression after '+'`
- [ ] `x = (5 + 2` → error naming the missing `)`, no crash
- [ ] `x = 5 # 2` → `unexpected character '#'`, no crash
- [ ] `= 5` → `expected identifier`, no crash
- [ ] No sample (valid or broken) segfaults or leaks unbounded memory (checked with a debug build / AddressSanitizer if available)
- [ ] Program exits with status 0 on valid input, nonzero on any reported error

---

## M5 — Test Suite, Build Polish & Docs

**Goal:** Everything in spec §12 (Definition of Done) is mechanically verifiable, and the project is presentable for the demo.

**Files added/changed:**
- `tests/run_tests.sh` (or a `test` target in `Makefile`) — runs every file in `tests/samples/`, compares actual vs. expected output, prints pass/fail summary
- `tests/samples/*.expected` — expected output per sample (one per input file)
- `README.md` — build/run instructions, project structure, example usage (spec §10)
- `Makefile` — add `test` target

**Acceptance criteria:**
- [ ] `make test` (or `./tests/run_tests.sh`) runs all valid + broken samples and reports 100% pass
- [ ] Every item in spec §12 Definition of Done is checked off
- [ ] `README.md` lets a new team member build and run the project from a clean checkout
- [ ] Fresh clone → `make && make test` succeeds with no manual steps

---

## M6 — Stretch: `if` / `while` Control Structures

**Goal:** Extend grammar/lexer/parser/AST to support conditionals and loops, per spec §2 stretch goals.

**Files changed:**
- `src/token.h` / `src/lexer.c` — keyword tokens already scaffolded in M2, now consumed
- `src/ast.h` / `src/ast.c` — new node types (`NODE_IF`, `NODE_WHILE`, `NODE_BLOCK`)
- `src/parser.h` / `src/parser.c` — `parse_if()`, `parse_while()`, block/statement-list parsing
- `tests/samples/valid_if_01.txt`, `valid_while_01.txt` (+ `.expected`)

**Acceptance criteria:**
- [ ] `if (cond) { ... }` and `while (cond) { ... }` parse into correct AST shapes
- [ ] Nested/combined control structures parse correctly
- [ ] Malformed control structures produce clear errors, not crashes
- [ ] New samples pass under `make test`

---

## M7 — Stretch: Tree-Walking Interpreter

**Goal:** Evaluate the AST and report results, with a symbol table and basic semantic checks, per spec §2/§13.

**Files added/changed:**
- `src/interpreter.h` / `src/interpreter.c` — `eval()`, symbol table (name → int value)
- `src/main.c` — optional `--run` flag (or always-on) to evaluate after parsing
- `tests/samples/eval_01.txt` (+ `.expected` showing evaluated output)

**Acceptance criteria:**
- [ ] Assignment expressions evaluate and store correct integer values
- [ ] Referencing an undeclared variable reports a semantic error (line/col) instead of crashing
- [ ] `if`/`while` (if M6 done) evaluate with correct control flow
- [ ] All interpreter samples pass under `make test`

---

## Working Agreement

- Implement strictly in order: M1 → M2 → M3 → M4 → M5, then M6/M7 only if time allows (per spec §13).
- Each milestone ends with `make && make test` (once M5 exists) passing before starting the next.
- No milestone modifies files "ahead" of itself except where explicitly listed above (e.g., M2 pre-scaffolds keyword tokens for M6).
