#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "error.h"

typedef struct {
    Token *tokens;
    int count;
    int pos;
} Parser;

static Token *current(Parser *p) {
    if (p->pos < p->count) return &p->tokens[p->pos];
    return &p->tokens[p->count - 1]; /* EOF */
}

static Token *advance(Parser *p) {
    Token *t = current(p);
    if (p->pos < p->count - 1) p->pos++;
    return t;
}

static int is_operator(Token *t, char op) {
    return t->type == TOKEN_OPERATOR && t->lexeme[0] == op && t->lexeme[1] == '\0';
}

static ASTNode *make_number_node(Token *t) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = NODE_NUMBER;
    node->value = atoi(t->lexeme);
    return node;
}

static ASTNode *make_identifier_node(Token *t) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = NODE_IDENTIFIER;
    strncpy(node->name, t->lexeme, sizeof(node->name) - 1);
    return node;
}

static ASTNode *make_binary_node(char op, ASTNode *left, ASTNode *right) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = NODE_BINARY_OP;
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

static ASTNode *parse_expr(Parser *p);
static ASTNode *parse_term(Parser *p, const char *after);

static ASTNode *parse_factor(Parser *p, const char *after) {
    Token *t = current(p);

    if (t->type == TOKEN_NUMBER) {
        advance(p);
        return make_number_node(t);
    }
    if (t->type == TOKEN_IDENTIFIER) {
        advance(p);
        return make_identifier_node(t);
    }
    if (t->type == TOKEN_LPAREN) {
        advance(p);
        ASTNode *inner = parse_expr(p);
        if (current(p)->type != TOKEN_RPAREN) {
            report_error(current(p)->line, current(p)->column, "expected ')'");
            return inner;
        }
        advance(p);
        return inner;
    }

    if (after) {
        report_error(t->line, t->column, "expected expression after '%s'", after);
    } else {
        report_error(t->line, t->column, "expected expression");
    }
    return NULL;
}

static ASTNode *parse_term(Parser *p, const char *after) {
    ASTNode *node = parse_factor(p, after);
    while (!had_error() && (is_operator(current(p), '*') || is_operator(current(p), '/'))) {
        Token *op = advance(p);
        ASTNode *right = parse_factor(p, op->lexeme);
        node = make_binary_node(op->lexeme[0], node, right);
    }
    return node;
}

static ASTNode *parse_expr(Parser *p) {
    ASTNode *node = parse_term(p, NULL);
    while (!had_error() && (is_operator(current(p), '+') || is_operator(current(p), '-'))) {
        Token *op = advance(p);
        ASTNode *right = parse_term(p, op->lexeme);
        node = make_binary_node(op->lexeme[0], node, right);
    }
    return node;
}

static ASTNode *parse_statement(Parser *p) {
    Token *ident = current(p);
    if (ident->type != TOKEN_IDENTIFIER) {
        report_error(ident->line, ident->column, "expected identifier");
        return NULL;
    }
    advance(p);

    Token *eq = current(p);
    if (!is_operator(eq, '=')) {
        report_error(eq->line, eq->column, "expected '='");
        return NULL;
    }
    advance(p);

    ASTNode *expr = parse_expr(p);

    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = NODE_ASSIGN;
    strncpy(node->name, ident->lexeme, sizeof(node->name) - 1);
    node->left = make_identifier_node(ident);
    node->right = expr;
    return node;
}

ASTNode *parse(Token *tokens, int token_count) {
    Parser p = { tokens, token_count, 0 };
    return parse_statement(&p);
}
