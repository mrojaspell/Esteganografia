CC=gcc
CFLAGS=-Wall -std=c99 -pedantic
LDFLAGS=-lm -lssl -lcrypto
DEBUGFLAGS=-g
INCLUDE=-Isrc/include
DIRS := src src/embed src/extract
SOURCES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS = $(SOURCES:.c=.o)
BIN_DIRECTORY=./bin
BIN_FILE=./bin/stegobmp

all: $(BIN_FILE)

$(BIN_FILE): $(OBJS)
	@mkdir -p $(BIN_DIRECTORY)
	@$(CC) $(CFLAGS) $(DEBUGFLAGS) $(INCLUDE) $(OBJS) $(LDFLAGS) -o $(BIN_FILE)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	@rm -rf $(BIN_DIRECTORY) $(OBJS)

.PHONY: clean all
