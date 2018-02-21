PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

CC = g++
CFLAGS = -std=c++14 -m64 -Ofast -DNDEBUG -Wall -pedantic -flto -march=native
FILES = firstchess.o uci.o
NAME = StartChess

all: firstchess.c $(FILES)
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

firstchess.o: firstchess.c
	$(CC) $(CFLAGS) -c Firstchess.c

uci.o: uci.c
	$(CC) $(CFLAGS) -c uci.c

clean:
	rm -rf *.o $(NAME)
