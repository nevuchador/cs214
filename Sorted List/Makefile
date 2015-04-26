TARGET = sl
CC = gcc
CFLAGS =-Wall -g -Werror -std=c99 -pedantic
EXEOBJ = main.o
LIBOBJ = sorted-list.o
LIBSL = libsl.a

default: $(TARGET)
all: default

HEADERS = sorted-list.h

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBSL): $(LIBOBJ)
	ar rc $(LIBSL) $(LIBOBJ)

$(TARGET): $(EXEOBJ) $(LIBSL)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f $(LIBSL)
	-rm -f *~
