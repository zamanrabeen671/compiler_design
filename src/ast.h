#ifndef AST_H
#define AST_H

typedef enum {
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_BINARY_OP,
    NODE_ASSIGN
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char op;                    /* '+', '-', '*', '/' */
    int value;                  /* for NODE_NUMBER */
    char name[64];               /* for NODE_IDENTIFIER / assignment target */
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Implemented in M3 */
void print_ast(ASTNode *node, int depth);
void free_ast(ASTNode *node);

#endif
