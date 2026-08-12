#include <stdio.h>
#include <stdarg.h>
#include "error.h"

static int error_flag = 0;

void report_error(int line, int column, const char *fmt, ...) {
    char message[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    fflush(stdout);
    fprintf(stderr, "Error (line %d, col %d): %s\n", line, column, message);
    error_flag = 1;
}

int had_error(void) {
    return error_flag;
}
