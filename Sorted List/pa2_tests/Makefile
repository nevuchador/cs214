CC = gcc
CFLAGS = -Wall -g -pedantic -std=c99

all: pa2_test

#sorted-list.o: sorted-list.c
#	$(CC) $(FLAGS) -c sorted-list.c
	
#libsl.a: sorted-list.o
#	ar rvs libsl.a sorted-list.o

pa2_test: pa2_test.c libsl.a
	$(CC) $(FLAGS) -o $@ $^

clean:
	@rm -f pa2_test
