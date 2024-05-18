CC=gcc
CCFLAGS=-Wall -std=c99 -pedantic -lm
SOURCES=$(wildcard src/*.c)
BIN_DIRECTORY=./bin
BIN_FILE=./bin/stegobmp

all:
	@mkdir -p $(BIN_DIRECTORY)
	@$(CC) $(CCFLAGS) $(SOURCES) -o $(BIN_FILE)

clean:
	@rm -rf $(BIN_DIRECTORY)

PHONY: clean all