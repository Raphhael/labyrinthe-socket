/**********************************
 * CONSTANTES
 *********************************/
#include <stdlib.h>

#ifndef CONSTANTS
#define CONSTANTS

        #ifdef DEBUG_ON
                #define DEBUG(txt...) fprintf(stderr, txt)
        #else
                #define DEBUG(txt...) ;
        #endif

        /* BDD */
        #define LABYRINTHE_NOM_LEN      250
        #define NIVEAU_NOM_LEN          250
        
        
        #define	DEFAULT_LVL 	{ "Débutant", "Intermediaire", "Confirmé", "Avancé", "Expert" }
        #define	NB_DEFAULT_LVL	5
        #define LEN_DEFAULT_LVL	50

        #define LIGNE_MAX       50
        #define LIGNE_MIN       3
        #define COLONNE_MAX     100
        #define COLONNE_MIN     7
        
        #define MUR_X           '+'
        #define MUR_V           '|'
        #define MUR_H           '-'
        #define ENTREE          '>'
        #define BLANC           ' '
        
        /* AUTRE */
        #define PSEUDO_LEN      8   /* Longueur d'un pseudo */
        #define NOM_LEN         8   /* Longueur d'un pseudo */
        
        #define VERIF_CRITIQUE(var, val, errmsg)        \
                if((var) == val) {                      \
                        perror(errmsg);                 \
                        exit(EXIT_FAILURE);             \
                }
        #define VERIF_CRITIQUE_ZERO(var, errmsg)        \
                if((var)) {VERIF_CRITIQUE(1, 1, errmsg);} 
        
        #define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
        
        #define MENU_HELP                                                                \
                mvprintw(LINES - 2, 0, "F5 pour actualiser, F4 pour retourner au menu"); 
        
        /*********************************************************************/
        /*************** LABELS **********************************************/
        /*********************************************************************/
        #define MENU_CREATE_PARTY       "Creer une partie"
        #define MENU_JOIN_PARTY         "Rejoindre une partie"
        #define MENU_EXIT               "Quitter"
        #define MENU_RETOUR             "Retour au menu"
        #define MENU_NO_RESULTS         "Pas de résultats"
        #define MENU_HIST_PARTIE_PSEUDO "Historique d'un joueur"
        #define MENU_HIST_PARTIE_LAB    "Historique d'un labyrinthe"
        #define MENU_OBSERVE_GAME       "Observer une partie"
        
        
        /* Constantes */
        
        #define OBSERVER_PARTIE      1
        #define REJOINDRE_PARTIE     2
        #define HIST_FROM_PSEUDO     1
        #define HIST_FROM_PSEUDO     1
        #define HIST_FROM_LAB        2
        
#endif

