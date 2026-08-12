#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

#define MAX_DEPTH 64
static int is_last_at[MAX_DEPTH];

static void print_label(ASTNode *node) {
    switch (node->type) {
        case NODE_NUMBER:
            printf("Number: %d\n", node->value);
            break;
        case NODE_IDENTIFIER:
            printf("Identifier: %s\n", node->name);
            break;
        case NODE_BINARY_OP:
            printf("BinaryExpr (%c)\n", node->op);
            break;
        case NODE_ASSIGN:
            printf("Assign\n");
            break;
    }
}

void print_ast(ASTNode *node, int depth) {
    if (!node) return;

    if (depth == 0) {
        print_label(node);
    } else {
        int i;
        for (i = 0; i < depth - 1; i++) {
            printf(is_last_at[i] ? "    " : "\xe2\x94\x82   ");
        }
        printf(is_last_at[depth - 1] ? "\xe2\x94\x94\xe2\x94\x80\xe2\x94\x80 " : "\xe2\x94\x9c\xe2\x94\x80\xe2\x94\x80 ");
        print_label(node);
    }

    if (depth >= MAX_DEPTH - 1) return;

    if (node->left) {
        is_last_at[depth] = (node->right == NULL);
        print_ast(node->left, depth + 1);
    }
    if (node->right) {
        is_last_at[depth] = 1;
        print_ast(node->right, depth + 1);
    }
}

void free_ast(ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}
