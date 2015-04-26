CC = gcc
CFLAGS = -Wall -Werror -std=gnu99 -pedantic -I. -I./include
DEPS = bank.h
CLIENT_OBJ = client.o 
SERVER_OBJ = server.o bank.o
LIBS = -lpthread

all: client server

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)


client: $(CLIENT_OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

server: $(SERVER_OBJ) 
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f client server *.o
