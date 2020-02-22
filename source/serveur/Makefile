OBJS		= main.o ../mtp/mtp.o ../mtp/push.o ../bdd/serveur.o ../bdd/util.o gestion_connexions.o creation_serveur.o liste_chainee.o
OUT		= ../../bin/maze_ser

CC		= gcc
FLAGS		= -g -static -c -Wall
LFLAGS		= -g -Wall -pthread 
ECPGFL		= -I /usr/include/postgresql -lecpg

all: bdd mtp gestion_connexions.o creation_serveur.o liste_chainee.o main.o
	$(CC) $(LFLAGS) -o $(OUT) $(OBJS) $(ECPGFL)
	rm -rf _*

bdd:
	$(MAKE) -C ../bdd serveur

main.o: main.c ../constants.h structures.h gestion_connexions.h creation_serveur.h liste_chainee.h
	$(CC) $(FLAGS) main.c

gestion_connexions.o: gestion_connexions.c ../constants.h ../mtp/mtp.h ../mtp/push.h ../bdd/util.h structures.h liste_chainee.h
	$(CC) $(FLAGS) gestion_connexions.c

creation_serveur.o: creation_serveur.c structures.h liste_chainee.h gestion_connexions.h 
	$(CC) $(FLAGS) creation_serveur.c

liste_chainee.o: liste_chainee.c structures.h ../constants.h
	$(CC) $(FLAGS) liste_chainee.c



mtp:
	$(MAKE) -C ../mtp

mrproper: clean
	rm -rf $(OUT)

clean:
	rm -rf $(OBJS)


