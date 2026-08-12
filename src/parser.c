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

static int is_operator(Token *t, const char *op) {
    return t->type == TOKEN_OPERATOR && strcmp(t->lexeme, op) == 0;
}

static int is_comparison(Token *t) {
    return t->type == TOKEN_OPERATOR &&
        (strcmp(t->lexeme, "<") == 0 || strcmp(t->lexeme, ">") == 0 ||
         strcmp(t->lexeme, "<=") == 0 || strcmp(t->lexeme, ">=") == 0 ||
         strcmp(t->lexeme, "==") == 0 || strcmp(t->lexeme, "!=") == 0);
}

static int is_keyword(Token *t, const char *kw) {
    return t->type == TOKEN_KEYWORD && strcmp(t->lexeme, kw) == 0;
}

static ASTNode *make_number_node(Token *t) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = NODE_NUMBER;
    node->value = atoi(t->lexeme);
    node->line = t->line;
    node->column = t->column;
    return node;
}

static ASTNode *make_identifier_node(Token *t) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = NODE_IDENTIFIER;
    strncpy(node->name, t->lexeme, sizeof(node->name) - 1);
    node->line = t->line;
    node->column = t->column;
    return node;
}

static ASTNode *make_op_node(NodeType type, Token *pos, const char *op, ASTNode *left, ASTNode *right) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = type;
    strncpy(node->op, op, sizeof(node->op) - 1);
    node->line = pos->line;
    node->column = pos->column;
    node->left = left;
    node->right = right;
    return node;
}

static ASTNode *parse_expr(Parser *p);
static ASTNode *parse_term(Parser *p, const char *after);
static ASTNode *parse_statement(Parser *p);

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
    while (!had_error() && (is_operator(current(p), "*") || is_operator(current(p), "/"))) {
        Token *op = advance(p);
        ASTNode *right = parse_factor(p, op->lexeme);
        node = make_op_node(NODE_BINARY_OP, op, op->lexeme, node, right);
    }
    return node;
}

static ASTNode *parse_expr(Parser *p) {
    ASTNode *node = parse_term(p, NULL);
    while (!had_error() && (is_operator(current(p), "+") || is_operator(current(p), "-"))) {
        Token *op = advance(p);
        ASTNode *right = parse_term(p, op->lexeme);
        node = make_op_node(NODE_BINARY_OP, op, op->lexeme, node, right);
    }
    return node;
}

static ASTNode *parse_condition(Parser *p) {
    ASTNode *left = parse_expr(p);
    if (!had_error() && is_comparison(current(p))) {
        Token *op = advance(p);
        ASTNode *right = parse_expr(p);
        return make_op_node(NODE_COMPARISON, op, op->lexeme, left, right);
    }
    return left;
}

static ASTNode *parse_block(Parser *p) {
    if (current(p)->type != TOKEN_LBRACE) {
        report_error(current(p)->line, current(p)->column, "expected '{'");
        return NULL;
    }
    advance(p);

    ASTNode *head = NULL;
    ASTNode **tail = &head;

    while (!had_error() && current(p)->type != TOKEN_RBRACE && current(p)->type != TOKEN_EOF) {
        Token *stmt_start = current(p);
        ASTNode *stmt = parse_statement(p);
        ASTNode *link = calloc(1, sizeof(ASTNode));
        link->type = NODE_BLOCK;
        link->line = stmt_start->line;
        link->column = stmt_start->column;
        link->left = stmt;
        *tail = link;
        tail = &link->right;
    }

    if (current(p)->type != TOKEN_RBRACE) {
        report_error(current(p)->line, current(p)->column, "expected '}'");
        return head;
    }
    advance(p);
    return head;
}

static ASTNode *parse_if(Parser *p) {
    Token *kw = advance(p); /* consume 'if' */

    if (current(p)->type != TOKEN_LPAREN) {
        report_error(current(p)->line, current(p)->column, "expected '(' after 'if'");
        return NULL;
    }
    advance(p);

    ASTNode *cond = parse_condition(p);

    if (current(p)->type != TOKEN_RPAREN) {
        report_error(current(p)->line, current(p)->column, "expected ')'");
        return make_op_node(NODE_IF, kw, "", cond, NULL);
    }
    advance(p);

    ASTNode *body = parse_block(p);
    return make_op_node(NODE_IF, kw, "", cond, body);
}

static ASTNode *parse_while(Parser *p) {
    Token *kw = advance(p); /* consume 'while' */

    if (current(p)->type != TOKEN_LPAREN) {
        report_error(current(p)->line, current(p)->column, "expected '(' after 'while'");
        return NULL;
    }
    advance(p);

    ASTNode *cond = parse_condition(p);

    if (current(p)->type != TOKEN_RPAREN) {
        report_error(current(p)->line, current(p)->column, "expected ')'");
        return make_op_node(NODE_WHILE, kw, "", cond, NULL);
    }
    advance(p);

    ASTNode *body = parse_block(p);
    return make_op_node(NODE_WHILE, kw, "", cond, body);
}

static ASTNode *parse_assign_stmt(Parser *p) {
    Token *ident = current(p);
    if (ident->type != TOKEN_IDENTIFIER) {
        report_error(ident->line, ident->column, "expected identifier");
        return NULL;
    }
    advance(p);

    Token *eq = current(p);
    if (!is_operator(eq, "=")) {
        report_error(eq->line, eq->column, "expected '='");
        return NULL;
    }
    advance(p);

    ASTNode *expr = parse_expr(p);

    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = NODE_ASSIGN;
    strncpy(node->name, ident->lexeme, sizeof(node->name) - 1);
    node->line = ident->line;
    node->column = ident->column;
    node->left = make_identifier_node(ident);
    node->right = expr;
    return node;
}

static ASTNode *parse_statement(Parser *p) {
    Token *t = current(p);
    if (is_keyword(t, "if")) return parse_if(p);
    if (is_keyword(t, "while")) return parse_while(p);
    return parse_assign_stmt(p);
}

ASTNode *parse(Token *tokens, int token_count) {
    Parser p = { tokens, token_count, 0 };
    return parse_statement(&p);
}
