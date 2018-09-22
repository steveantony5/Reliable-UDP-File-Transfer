#makefile for server

CC = gcc -Wall

server : server.c

	$(CC) server.c -o server

clean:
	rm -f server
