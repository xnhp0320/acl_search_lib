CC     = gcc
CFLAGS = -g -Wall -Werror -O2


all: hs

hs: main.c hs.c utils.c heap.c
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm -f hs.o hs 
