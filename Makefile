CC     = gcc
#GCOV_CCFLAGS = -fprofile-arcs -ftest-coverage
GCOV_OUTPUT = *.gcda *.gcno *.gcov 

#CFLAGS = -g -Wall -Werror -O2

.PHONY: all
all: libhs.a

test: CFLAGS = -g -Wall -Werror -O2 $(GCOV_CCFLAGS)
test_rule: CFLAGS = -g -Wall -Werror -O0 $(GCOV_CCFLAGS)
all: CFLAGS = -g -Wall -Werror -O2 -DLIB 

test: main.o hs.o utils.o heap.o mem.o rule.o
	$(CC) $(CFLAGS) -o $@ $^
	#./test -r fw1K
	#gcov hs.c

test_rule:test_rule.o hs.o utils.o heap.o mem.o rule.o
	$(CC) $(CFLAGS) -o $@ $^


libhs.a: hs.o utils.o heap.o mem.o rule.o
	$(AR) rvs $@ $^


clean:
	rm -f libhs.a test *.o $(GCOV_OUTPUT)
