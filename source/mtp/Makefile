ECPGFL	= -I /usr/include/postgresql/
CCFL	= -g -c -Wall
CC	= gcc

all: mtp.o push.o get.o

mtp.o: mtp.c mtp.h
	$(CC) $(CCFL) mtp.c
	
push.o: mtp.h push.h push.c 
	$(CC) $(CCFL) push.c
	
get.o: mtp.h get.h get.c
	$(CC) $(CCFL) get.c

clean:
	rm -f *.o
