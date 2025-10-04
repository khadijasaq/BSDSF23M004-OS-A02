# Makefile for OS Assignment 02: ls clone (Feature 1)

CC = gcc
CFLAGS = -Wall
SRC = src/lsv1.0.0.c
OBJ = obj/lsv1.0.0.o
BIN = bin/ls

# Default target
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

clean:
	rm -f $(OBJ) $(BIN)
