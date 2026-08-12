#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"
#include "error.h"

#define MAX_WHILE_ITERATIONS 100000

typedef struct {
    char name[64];
    int value;
} Symbol;

static Symbol *symbols = NULL;
static int symbol_count = 0;
static int symbol_capacity = 0;

static int find_symbol(const char *name) {
    int i;
    for (i = 0; i < symbol_count; i++) {
        if (strcmp(symbols[i].name, name) == 0) return i;
    }
    return -1;
}

static void set_symbol(const char *name, int value) {
    int idx = find_symbol(name);
    if (idx >= 0) {
        symbols[idx].value = value;
        return;
    }
    if (symbol_count == symbol_capacity) {
        symbol_capacity = symbol_capacity == 0 ? 16 : symbol_capacity * 2;
        symbols = realloc(symbols, sizeof(Symbol) * (size_t)symbol_capacity);
    }
    strncpy(symbols[symbol_count].name, name, sizeof(symbols[symbol_count].name) - 1);
    symbols[symbol_count].value = value;
    symbol_count++;
}

static int eval_expr(ASTNode *node) {
    if (!node || had_error()) return 0;

    switch (node->type) {
        case NODE_NUMBER:
            return node->value;

        case NODE_IDENTIFIER: {
            int idx = find_symbol(node->name);
            if (idx < 0) {
                report_error(node->line, node->column, "undeclared variable '%s'", node->name);
                return 0;
            }
            return symbols[idx].value;
        }

        case NODE_BINARY_OP: {
            int left = eval_expr(node->left);
            int right = eval_expr(node->right);
            if (had_error()) return 0;

            if (node->op[0] == '+') return left + right;
            if (node->op[0] == '-') return left - right;
            if (node->op[0] == '*') return left * right;
            if (node->op[0] == '/') {
                if (right == 0) {
                    report_error(node->line, node->column, "division by zero");
                    return 0;
                }
                return left / right;
            }
            return 0;
        }

        case NODE_COMPARISON: {
            int left = eval_expr(node->left);
            int right = eval_expr(node->right);
            if (had_error()) return 0;

            if (strcmp(node->op, "<") == 0) return left < right;
            if (strcmp(node->op, ">") == 0) return left > right;
            if (strcmp(node->op, "<=") == 0) return left <= right;
            if (strcmp(node->op, ">=") == 0) return left >= right;
            if (strcmp(node->op, "==") == 0) return left == right;
            if (strcmp(node->op, "!=") == 0) return left != right;
            return 0;
        }

        default:
            return 0;
    }
}

static void eval_block(ASTNode *block);

static void eval_stmt(ASTNode *node) {
    if (!node || had_error()) return;

    switch (node->type) {
        case NODE_ASSIGN: {
            int value = eval_expr(node->right);
            if (had_error()) return;
            set_symbol(node->left->name, value);
            printf("%s = %d\n", node->left->name, value);
            return;
        }

        case NODE_IF: {
            int cond = eval_expr(node->left);
            if (had_error()) return;
            if (cond != 0) eval_block(node->right);
            return;
        }

        case NODE_WHILE: {
            int iterations = 0;
            while (!had_error()) {
                int cond = eval_expr(node->left);
                if (had_error() || !cond) break;

                eval_block(node->right);

                iterations++;
                if (iterations >= MAX_WHILE_ITERATIONS) {
                    report_error(node->line, node->column, "while loop exceeded %d iterations", MAX_WHILE_ITERATIONS);
                    break;
                }
            }
            return;
        }

        default:
            return;
    }
}

static void eval_block(ASTNode *block) {
    ASTNode *link;
    for (link = block; link && !had_error(); link = link->right) {
        eval_stmt(link->left);
    }
}

void eval_program(ASTNode *root) {
    free(symbols);
    symbols = NULL;
    symbol_count = 0;
    symbol_capacity = 0;

    eval_stmt(root);

    free(symbols);
    symbols = NULL;
    symbol_count = 0;
    symbol_capacity = 0;
}
