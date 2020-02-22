#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "../constants.h"
#include "../bdd/util.h"
#include "structures.h"
#include "liste_chainee.h"
#include "gestion_connexions.h"
#include "creation_serveur.h"
#ifdef DEBUG_ON
        #include <mcheck.h>
#endif


#define PORT 6666

struct liste clients;

void    on_sortie() {
        bdd_disconnect();
}

void    clean_exit(int status) {
        exit(EXIT_FAILURE);
}

void    sigint_to_exit() {
        struct sigaction sa;
        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_handler = clean_exit;
        VERIF_CRITIQUE(sigaction(SIGINT, &sa, NULL), -1, "Impossible d'installer sigint handler");
}
/*
 * main.c
 * argc[1] : port
 */
int main(int argc, char *argv[]) {
        #ifdef DEBUG_ON
                mtrace();
        #endif
        
        sigint_to_exit();
        
        if(atexit(on_sortie))
                fprintf(stderr, "Impossible d'installer le gestionnaire de fin \"on_sortie\".\n");
        
        liste_init(sizeof(struct s_client), &clients);
        
        bdd_connect();
        
        /* Initialisation du srv */
        creation_serveur(argc < 2 ? PORT : atoi(argv[1]));
        
        /* Autre allocation de ressources */
        init_connexions();
        
        /* Attente des connexions */
        start();
        
        return EXIT_SUCCESS;
}

