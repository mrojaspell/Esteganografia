CC=gcc
CFLAGS=-Wall -std=c99 -pedantic -g $(INCLUDE)
LDFLAGS=-lm -lssl -lcrypto
INCLUDE=-Isrc/include
DIRS := src src/embed src/extract src/embed/lsbi_pattern_inversion
SOURCES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS := $(SOURCES:.c=.o)
BIN_DIRECTORY=./bin
BIN_FILE=./bin/stegobmp

all: $(BIN_FILE)

$(BIN_FILE): $(OBJS)
	@mkdir -p $(BIN_DIRECTORY)
	@$(CC) $(OBJS) $(LDFLAGS) -o $(BIN_FILE)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BIN_DIRECTORY) $(OBJS)

.PHONY: clean all
