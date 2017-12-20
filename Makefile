CC     = gcc
GCOV_CCFLAGS = -fprofile-arcs -ftest-coverage
GCOV_OUTPUT = *.gcda *.gcno *.gcov 

CFLAGS = -g -Wall -Werror -O2

.PHONY: all
all: hs 

test: CFLAGS = -g -Wall -Werror -O2 $(GCOV_CCFLAGS)
all: CFLAGS = -g -Wall -Werror -O2  

test: main.c hs.c utils.c heap.c mem.c
	$(CC) $(CFLAGS) -o $@ $^
	./test -r acl1
	gcov hs.c

hs: hs.c utils.c heap.c mem.c
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm -f hs test $(GCOV_OUTPUT)
