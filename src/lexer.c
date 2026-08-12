#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "error.h"

static const char *KEYWORDS[] = { "if", "while" };
static const int KEYWORD_COUNT = 2;

static int is_keyword(const char *lexeme) {
    int i;
    for (i = 0; i < KEYWORD_COUNT; i++) {
        if (strcmp(lexeme, KEYWORDS[i]) == 0) return 1;
    }
    return 0;
}

typedef struct {
    Token *tokens;
    int count;
    int capacity;
} TokenList;

static void push_token(TokenList *list, TokenType type, const char *lexeme, int line, int column) {
    if (list->count == list->capacity) {
        list->capacity = list->capacity == 0 ? 64 : list->capacity * 2;
        list->tokens = realloc(list->tokens, sizeof(Token) * (size_t)list->capacity);
    }
    Token *t = &list->tokens[list->count++];
    t->type = type;
    strncpy(t->lexeme, lexeme, sizeof(t->lexeme) - 1);
    t->lexeme[sizeof(t->lexeme) - 1] = '\0';
    t->line = line;
    t->column = column;
}

static void push_word(TokenList *list, const char *source, size_t start, size_t length, int line, int col) {
    char lexeme[64];
    size_t copy_len = length < sizeof(lexeme) - 1 ? length : sizeof(lexeme) - 1;
    memcpy(lexeme, source + start, copy_len);
    lexeme[copy_len] = '\0';

    TokenType type = isdigit((unsigned char)lexeme[0])
        ? TOKEN_NUMBER
        : (is_keyword(lexeme) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER);
    push_token(list, type, lexeme, line, col);
}

Token *tokenize(const char *source, int *token_count) {
    TokenList list = { NULL, 0, 0 };
    int line = 1;
    int col = 1;
    size_t i = 0;
    size_t len = strlen(source);

    while (i < len) {
        char c = source[i];

        if (c == '\n') {
            line++;
            col = 1;
            i++;
            continue;
        }
        if (isspace((unsigned char)c)) {
            col++;
            i++;
            continue;
        }

        int start_col = col;

        if (isalpha((unsigned char)c) || c == '_') {
            size_t start = i;
            while (i < len && (isalnum((unsigned char)source[i]) || source[i] == '_')) {
                i++;
                col++;
            }
            push_word(&list, source, start, i - start, line, start_col);
            continue;
        }

        if (isdigit((unsigned char)c)) {
            size_t start = i;
            while (i < len && isdigit((unsigned char)source[i])) {
                i++;
                col++;
            }
            push_word(&list, source, start, i - start, line, start_col);
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=') {
            char lexeme[2] = { c, '\0' };
            push_token(&list, TOKEN_OPERATOR, lexeme, line, start_col);
            i++;
            col++;
            continue;
        }

        if (c == '(') {
            push_token(&list, TOKEN_LPAREN, "(", line, start_col);
            i++;
            col++;
            continue;
        }

        if (c == ')') {
            push_token(&list, TOKEN_RPAREN, ")", line, start_col);
            i++;
            col++;
            continue;
        }

        report_error(line, start_col, "unexpected character '%c'", c);
        i++;
        col++;
    }

    push_token(&list, TOKEN_EOF, "", line, col);

    *token_count = list.count;
    return list.tokens;
}

void free_tokens(Token *tokens) {
    free(tokens);
}
