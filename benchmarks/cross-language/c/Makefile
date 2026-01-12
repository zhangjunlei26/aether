.PHONY: all clean ping_pong

CC = gcc
CFLAGS = -O3 -march=native -pthread -Wall
LDFLAGS = -pthread

all: ping_pong

ping_pong: ping_pong.c
	$(CC) $(CFLAGS) ping_pong.c -o ping_pong $(LDFLAGS)

clean:
	rm -f ping_pong
