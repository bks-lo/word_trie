CC = gcc
CFLAG = -g -Wall -O3
DBG_CFLAG = -g -Wall -O0 -fprofile-arcs -ftest-coverage -fPIC
CINCLUDE = -I./
CLIB =
CFLAGS = $(CFLAG) $(CINCLUDE) $(CLIB)
DBG_CFLAGS = $(DBG_CFLAG) $(CINCLUDE) $(CLIB) -lcheck
SRC = debug.c mytrie.c
HEAD = 
TAGS = libmytrie.so
TAGS_TEST_MYTRIE = test_mytrie

.PHONY:all
all:$(TAGS)

$(TAGS): $(SRC) $(HEAD) Makefile
	$(CC) -shared -fPIC $(CFLAGS) -o $(TAGS) $(SRC)

$(TAGS_TEST_MYTRIE): $(SRC) test_mytrie.c $(HEAD) Makefile
	$(CC) -DUNITTEST_MYTRIE $(DBG_CFLAGS) -o $(TAGS_TEST_MYTRIE) $(SRC)
	./$(TAGS_TEST_MYTRIE)

clean:
	rm -f *.o *.gcda *.gcno *.gcov *.html $(TAGS) $(TAGS_TEST_MYTRIE)
	
install:
	cp $(TAGS) /usr/lib/

