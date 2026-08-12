#ifndef ERROR_H
#define ERROR_H

typedef struct {
    int line;
    int column;
    char message[256];
} CompilerError;

void report_error(int line, int column, const char *fmt, ...);
int had_error(void);

#endif
