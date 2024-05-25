CC=gcc
CCFLAGS=-Wall -std=c99 -pedantic -lm
DEBUGFLAGS=-g
INCLUDE = -Isrc/include
DIRS := src src/embed src/extract
SOURCES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS = $(SRCS:.c=.o)
BIN_DIRECTORY=./bin
BIN_FILE=./bin/stegobmp

all: $(BIN_FILE)

$(BIN_FILE): $(OBJS)
	@mkdir -p $(BIN_DIRECTORY)
	@$(CC) $(CCFLAGS) $(DEBUGFLAGS) $(INCLUDE) $(SOURCES) -o $(BIN_FILE)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@


clean:
	@rm -rf $(BIN_DIRECTORY) $(OBJS)

PHONY: clean all