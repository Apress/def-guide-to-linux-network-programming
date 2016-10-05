CC		= gcc -Wall
COMMONDIR	= ../common
COMMONLIB	= $(COMMONDIR)/common.o

INCLUDES = -I$(COMMONDIR) -I./

LIBS = $(COMMONLIB)
LIBS += -lcrypto -lssl

CFLAGS += $(INCLUDES)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

BINS	= auth_client

all: libs $(BINS)

libs:
	make -C $(COMMONDIR)

auth_client.o: auth_client.c auth_client.h $(COMMONLIB)
	$(CC) -c $(CFLAGS) auth_client.c

auth_client: auth_client.o 
	$(CC) -o auth_client auth_client.o $(LIBS)

clean:
	rm -f *.o
	rm -f $(BINS)
	make -C $(COMMONDIR) clean

