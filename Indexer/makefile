TARGET = index
CC = gcc
CFLAGS = -Wall -Werror -std=c99 -pedantic -D_BSD_SOURCE
EXEOBJ = main.o
LIBOBJ = index.o sorted-list.o tokenizer.o

default: $(TARGET)
all: default

HEADERS = sorted-list.h tokenizer.h index.h

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@


$(TARGET): $(EXEOBJ) $(LIBOBJ)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f *~
