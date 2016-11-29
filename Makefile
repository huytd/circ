CC=gcc
SRC=main.c
BIN=irc
FLGS=-Wall
DEBUG_BIN=debug
DEBUG_FLG=-g -Wall

all: clean
	$(CC) $(FLGS) -o $(BIN) $(SRC)

debug: debug_clean
	$(CC) $(DEBUG_FLG) -o $(DEBUG_BIN) $(SRC)

clean:
	rm -rf $(BIN) *.o

debug_clean:
	rm -rf $(DEBUG_BIN)*
