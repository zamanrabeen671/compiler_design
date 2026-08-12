#ifndef AST_H
#define AST_H

typedef enum {
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_BINARY_OP,
    NODE_ASSIGN,
    NODE_COMPARISON, /* M6: <, >, <=, >=, ==, != */
    NODE_IF,          /* M6: left = condition, right = block */
    NODE_WHILE,       /* M6: left = condition, right = block */
    NODE_BLOCK        /* M6: left = statement, right = next block link (or NULL) */
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char op[3];                  /* "+", "-", "*", "/", "<", ">", "<=", ">=", "==", "!=" */
    int value;                  /* for NODE_NUMBER */
    char name[64];               /* for NODE_IDENTIFIER / assignment target */
    int line;                    /* M7: source position, for runtime error reporting */
    int column;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Implemented in M3 */
void print_ast(ASTNode *node, int depth);
void free_ast(ASTNode *node);

#endif
