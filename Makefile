CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
SRC = src/main.c src/lexer.c src/token.c src/parser.c src/ast.c src/error.c
OBJ = $(SRC:.c=.o)
TARGET = compilerfrontend

ifeq ($(OS),Windows_NT)
	RM = cmd /C del /Q /F
	CLEAN_OBJ = $(subst /,\,$(OBJ))
else
	RM = rm -f
	CLEAN_OBJ = $(OBJ)
endif

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	bash tests/run_tests.sh

clean:
	-$(RM) $(CLEAN_OBJ) $(TARGET) $(TARGET).exe

.PHONY: all clean test
