CFLAGS = -c -Wall
CC = gcc

all: server client

server:
	${CC} server.c player.c room.c testwin.c -o server

client:
	${CC} client.c interface.c close.c -o client `pkg-config --cflags --libs gtk+-3.0`

clean:
	rm -f *.o *~