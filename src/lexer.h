#ifndef LEXER_H
#define LEXER_H

#include "token.h"

/* Implemented in M2 */
Token *tokenize(const char *source, int *token_count);
void free_tokens(Token *tokens);

#endif
