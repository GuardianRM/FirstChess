PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

CC = g++
CFLAGS = -std=c++14 -m64 -Ofast -DNDEBUG -Wall -pedantic -flto -march=native
FILES = main.o game.o killer.o hash.o uci.o preparation.o printer.o search.o gamethread.o gameservices.o movegenerator.o timer.o goback.o figurecell.o bitmove.o movearray.o bitboard.o  magic.o option.o constants.o score.o
NAME = 1

all: Firstchess.c $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o $(NAME)

strip:
	strip $(NAME)

install:
	make -j4 clean
	make -j4 all
	-mkdir -p -m 755 $(BINDIR)
	-cp $(NAME) $(BINDIR)
	-strip $(BINDIR)/$(NAME)

uninstall:
	rm -rf $(BINDIR)/$(NAME)

Firstchess.o: Firstchess.c
	$(CC) $(CFLAGS) -c Firstchess.c

clean:
	rm -rf *.o $(NAME)
