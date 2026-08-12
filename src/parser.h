#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"

/* Implemented in M3 */
ASTNode *parse(Token *tokens, int token_count);

#endif
