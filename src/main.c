#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Could not open file: %s\n", path);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = malloc((size_t)size + 1);
    fread(buffer, 1, (size_t)size, f);
    buffer[size] = '\0';

    fclose(f);
    return buffer;
}

static void print_tokens(Token *tokens, int token_count) {
    int printed_on_line = 0;
    int current_line = -1;
    int i;

    printf("--- Tokens ---\n");
    for (i = 0; i < token_count; i++) {
        Token *t = &tokens[i];
        if (t->type == TOKEN_EOF) continue;

        if (t->line != current_line) {
            if (printed_on_line) printf("\n");
            current_line = t->line;
            printed_on_line = 0;
        }

        if (printed_on_line) printf(" ");

        switch (t->type) {
            case TOKEN_LPAREN:
                printf("LPAREN");
                break;
            case TOKEN_RPAREN:
                printf("RPAREN");
                break;
            case TOKEN_LBRACE:
                printf("LBRACE");
                break;
            case TOKEN_RBRACE:
                printf("RBRACE");
                break;
            default:
                printf("%s(%s)", token_type_name(t->type), t->lexeme);
                break;
        }
        printed_on_line = 1;
    }
    if (printed_on_line) printf("\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source-file> [--eval]\n", argv[0]);
        return 1;
    }

    int do_eval = (argc >= 3 && strcmp(argv[2], "--eval") == 0);

    char *source = read_file(argv[1]);

    int token_count = 0;
    Token *tokens = tokenize(source, &token_count);

    if (had_error()) {
        free_tokens(tokens);
        free(source);
        return 1;
    }

    print_tokens(tokens, token_count);

    ASTNode *ast = parse(tokens, token_count);

    if (had_error()) {
        free_ast(ast);
        free_tokens(tokens);
        free(source);
        return 1;
    }

    printf("\n--- AST ---\n");
    print_ast(ast, 0);

    if (do_eval) {
        printf("\n--- Evaluation ---\n");
        eval_program(ast);
    }

    int exit_code = had_error() ? 1 : 0;

    free_ast(ast);
    free_tokens(tokens);
    free(source);
    return exit_code;
}
