/********************************************************
 * Importe un fichier contenant des labyrinthes
 * dans la BDD.
 * syntaxe : ./import [nom_fichier]
 * Par défaut, nom_fichier = DEFAULT_FILENAME
 *
 * Le fichier doit être composé d'une suite de labyrinthe uniquement
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#include "../constants.h"
#include "import.h"
#include "../bdd/import.h"

#define DEFAULT_FILENAME        "labyrinthes.txt"

int     main(int argc, char *argv[]) {
        char    filename[NAME_MAX] = DEFAULT_FILENAME;
        int     fd;
        
        if(argc > 1)
                strcpy(filename, argv[1]);
        
        fd = open(filename, O_RDONLY);
        VERIF_CRITIQUE(fd, -1, "Impossible d'ouvrir le fichier ");
        
        bdd_init();
        parser_fichier(fd);
        bdd_exit();
        
        close(fd);
        
        return EXIT_SUCCESS;
}
