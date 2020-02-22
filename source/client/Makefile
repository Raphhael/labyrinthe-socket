OBJS		= ../mtp/mtp.o ../mtp/get.o menu.o choix_bonhomme.o historique_parties.o choix_labyrinthe.o rejoindre.o connexion.o client.o labyrinthe.o 
OUT		= ../../bin/maze_cli

DEPS_MTP	= ../mtp/mtp.h ../mtp/get.h

CC		= gcc
FLAGS		= -static -g -c -Wall
LFLAGS		= -pthread -g -Wall
ECPGFL		= -I /usr/include/postgresql -lecpg

all: client
	rm -f _*
	
mtp:
	$(MAKE) -C ../mtp

client: mtp client.o menu.o rejoindre.o connexion.o choix_labyrinthe.o choix_bonhomme.o labyrinthe.o historique_parties.o
	$(CC) $(LFLAGS) -o $(OUT) $(OBJS) -lmenu -lncurses

client.o: menu.o connexion.o  client.c ../constants.h ../mtp/mtp.h ../mtp/get.h
	$(CC) $(FLAGS) client.c

menu.o: menu.c ../constants.h choix_labyrinthe.h historique_parties.h rejoindre.h
	$(CC) $(FLAGS) menu.c
	
connexion.o: menu.o connexion.c ../constants.h
	$(CC) $(FLAGS) connexion.c
	
labyrinthe.o: labyrinthe.c ../constants.h ../mtp/mtp.h ../mtp/get.h
	$(CC) $(FLAGS) labyrinthe.c

choix_labyrinthe.o: menu.o choix_labyrinthe.c ../constants.h ../mtp/mtp.h ../mtp/get.h choix_bonhomme.h
	$(CC) $(FLAGS) choix_labyrinthe.c

choix_bonhomme.o: menu.o choix_bonhomme.c ../constants.h ../mtp/mtp.h ../mtp/get.h
	$(CC) $(FLAGS) choix_bonhomme.c

rejoindre.o: menu.o rejoindre.c ../constants.h ../mtp/mtp.h ../mtp/get.h choix_bonhomme.h
	$(CC) $(FLAGS) rejoindre.c

historique_parties.o: historique_parties.c ../constants.h ../mtp/mtp.h ../mtp/get.h
	$(CC) $(FLAGS) historique_parties.c

clean:
	rm -f $(OBJS) $(OUT) *.o
