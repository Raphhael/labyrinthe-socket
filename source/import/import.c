#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "../bdd/import.h"
#include "../constants.h"


int     trouver_1ere_ligne(int fd) {
        int     current_pos = lseek(fd, 0, SEEK_CUR),
                byte_lu,
                longueur;
        char    buffer_ligne[COLONNE_MAX],
                *buffer_2;
        
        byte_lu = read(fd, buffer_ligne, COLONNE_MAX);
        VERIF_CRITIQUE(byte_lu, -1, "trouver_1ere_ligne: Error read");
        if(byte_lu == 0)
                return -1;
        
        buffer_2 = strchr(buffer_ligne, '\n');
        
        
        if(buffer_2 == NULL) {
                fprintf(stderr, "trouver_1ere_ligne: Plus de caractère newline\n");
                return 0;
        }
        
        longueur = buffer_2 - buffer_ligne;
        
        if(longueur) {
                lseek(fd, current_pos, SEEK_SET);
                return longueur;
        }
        else {
                lseek(fd, current_pos+1, SEEK_SET);
                return trouver_1ere_ligne(fd);
        }
        
}


void    parser_fichier(int fd) {
        int     nb_ligne,
                nb_col,
                c_ligne,
                nb_lab = 0,
                bytes_lus = 0,
                i;
        char    buffer_ligne[COLONNE_MAX];
        
        
        
        while((nb_col = trouver_1ere_ligne(fd)) > 0) {
                if(nb_col < COLONNE_MIN) {
                        fprintf(stderr, "%d colonnes dans le %dème labyrinthe, \
                        au moins %d attendues.\n", nb_col, nb_lab, COLONNE_MIN);
                }
                nb_ligne = LIGNE_MAX;
                c_ligne = 0;
                ++nb_lab;
                
                creer_labyrinthe(nb_lab);
                do {
                        bytes_lus = read(fd, buffer_ligne, nb_col + 1);
                        if(bytes_lus != nb_col + 1)
                                fprintf(stderr, "%d bytes lu, %d attendus\n", bytes_lus ,nb_col);
                        
                        if(buffer_ligne[nb_col] != '\n') {
                                fprintf(stderr, "Fin anormale !\n");
                        }
                        buffer_ligne[nb_col] = '\0';
                        
                        ++c_ligne;
                        creer_ligne(c_ligne);
                        
                        if(c_ligne == 2 && buffer_ligne[0] != ENTREE) {
                                fprintf(stderr, "Pas d'entrée trouvée !\n");
                        }
                                
                        if(buffer_ligne[nb_col - 1] == ENTREE) {
                                nb_ligne = c_ligne + 1;
                                if(nb_ligne < LIGNE_MIN) {
                                        fprintf(stderr, "%d lignes dans le %dème labyrinthe, \
                                        au moins %d attendues.\n", nb_ligne, nb_lab, LIGNE_MIN);
                                }
                        }
                        for(i = 1; i <= nb_col; i++) {
                                creer_colonne(i, buffer_ligne[i-1]);
                        }
                } while(c_ligne < nb_ligne);
                printf("Labyrinthe N°%d (%d x %d) Fait!\n", nb_lab, nb_col, nb_ligne);
        }
}
