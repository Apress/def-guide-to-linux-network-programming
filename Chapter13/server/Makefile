CC		= gcc -Wall
COMMONDIR	= ../common
COMMONLIB	= $(COMMONDIR)/common.o

INCLUDES = -I$(COMMONDIR) -I./

LIBS = $(COMMONLIB)
LIBS += -lcrypto -lssl -lpam

CFLAGS += $(INCLUDES)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

BINS	= auth_server

all: libs $(BINS)

libs:
	make -C $(COMMONDIR)

auth_server.o: auth_server.c auth_server.h $(COMMONLIB)
	$(CC) -c $(CFLAGS) auth_server.c

auth_server: auth_server.o 
	$(CC) -o auth_server auth_server.o $(LIBS)

clean:
	rm -f *.o
	rm -f $(BINS)
	make -C $(COMMONDIR) clean

