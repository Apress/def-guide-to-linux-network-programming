CC = gcc -Wall

INCLUDES = -I./

CFLAGS += $(INCLUDES)

all: common.o

common.o: common.c common.h
	$(CC) -c $(CFLAGS) common.c

clean:
	rm -f common.o

