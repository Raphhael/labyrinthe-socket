OBJS		= import.o ../bdd/util.o ../bdd/import.o main.o
OUT		= ../../bin/import_labyrinthes

CC		= gcc
FLAGS		= -g -c -Wall
LFLAGS		= -g -Wall
ECPGFL		= -I /usr/include/postgresql -lecpg

all: import.o main.o bdd
	$(CC) $(LFLAGS) -o $(OUT) $(OBJS) $(ECPGFL)

bdd:
	$(MAKE) -C ../bdd import
	
import.o: import.c ../constants.h ../bdd/import.h
	$(CC) $(FLAGS) import.c

main.o: main.c ../constants.h import.h ../bdd/import.h
	$(CC) $(FLAGS) main.c
	
test: test.c
	ecpg -o _test.c test.c $(ECPGFLAGS)
	$(CC) _test.c -o test -g -Wall -I /usr/include/postgresql -lecpg

mrproper: clean
	rm -rf $(OUT)

clean:
	rm -rf $(OBJS)


