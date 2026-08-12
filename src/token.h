#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_KEYWORD,   /* if/while */
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[64];
    int line;
    int column;
} Token;

const char *token_type_name(TokenType type);

#endif
