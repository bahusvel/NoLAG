.PHONY: clean

CC = gcc
CFLAGS = -W -Wall -Wextra -O2 -g -std=c99 -pthread -Iinclude
LDFLAGS = -shared -ldl


all: clean socket

clean:
	rm -f *.o *.so binencrypt monitor core segment_test

socket:
	$(CC) $(CFLAGS) src/socket.c -o socket

run: clean socket
	sudo ./socket eno2
