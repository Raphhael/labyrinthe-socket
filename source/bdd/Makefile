ECPGFL	= -I /usr/include/postgresql/
CCFL	= -static -g -c -Wall -I /usr/include/postgresql -lecpg
CC	= gcc

import: util.o import.o
	

util: util.o
	
	
serveur: util.o serveur.o
	

import.o: import.c import.h ../constants.h util.h
	ecpg $(ECPGFL) -o _import.c import.c
	$(CC) $(CCFL) -o import.o _import.c

serveur.o: serveur.c serveur.h ../constants.h util.h
	ecpg $(ECPGFL) -o _serveur.c serveur.c
	$(CC) $(CCFL) -o serveur.o _serveur.c

util.o: util.c util.h ../constants.h
	ecpg $(ECPGFL) -o _util.c util.c
	$(CC) $(CCFL) -o util.o _util.c

mrproper: clean

clean:
	rm -f _* *.o

	
