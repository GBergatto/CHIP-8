CC=gcc
CFLAGS=-Wall -Wextra
LIBS=/usr/lib -lSDL2
INCLUDES=/usr/include/SDL2 -D_REENTRANT #.

SRC=src
OBJ=obj
SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

BINDIR=bin
BIN=$(BINDIR)/chip8

all:$(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) -I$(INCLUDES) -L$(LIBS)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(RM) -r $(BINDIR)/* $(OBJ)/*
