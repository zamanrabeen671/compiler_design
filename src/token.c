#include "token.h"

const char *token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER:     return "NUMBER";
        case TOKEN_OPERATOR:   return "OPERATOR";
        case TOKEN_LPAREN:     return "LPAREN";
        case TOKEN_RPAREN:     return "RPAREN";
        case TOKEN_LBRACE:     return "LBRACE";
        case TOKEN_RBRACE:     return "RBRACE";
        case TOKEN_KEYWORD:    return "KEYWORD";
        case TOKEN_EOF:        return "EOF";
    }
    return "UNKNOWN";
}
