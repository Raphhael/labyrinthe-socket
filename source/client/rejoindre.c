#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <menu.h>
#include <unistd.h>

#include "../constants.h"
#include "../mtp/mtp.h"
#include "../mtp/get.h"
#include "menu.h"
#include "choix_bonhomme.h"

#define REJOINDRE_FORMAT   /*pseudo*/" - Labyrinthe "/*numero*/


extern int sockfd;

static WINDOW *forRejoindre;
static int lab_id;
static char (*pseudoAdversaire)[PSEUDO_LEN];

void observer_on_action(char *name) {
        int scanne;
        
        memset(*pseudoAdversaire, 0, PSEUDO_LEN);
        
        if ((scanne = sscanf(name, "Lab %d - %s ", &lab_id, *pseudoAdversaire)) > 0) {
                DEBUG("On rejoint ! %s et %d\n", *pseudoAdversaire, lab_id);
                send_get_observe(sockfd, *pseudoAdversaire);
        }
        else 
                DEBUG("Pas réussi à lire : %s -> %d\n", name, scanne);
}

void rejoindre_on_action(char *name) {
        int scanne;
        
        if ((scanne = sscanf(name, "%s - Labyrinthe %d", *pseudoAdversaire, &lab_id)) > 1) {
                DEBUG("On envoi la sauce ! %s\n", *pseudoAdversaire);
                send_get_join(sockfd, *pseudoAdversaire);
        }
        else 
                DEBUG("Pas réussi à lire : %s %s %d -> %d\n", name, *pseudoAdversaire, lab_id, scanne);
}

/*******************************************************************************
 *********** Créer le tableau d'items et charger l'hist dans la BDD ************
 ******************************************************************************/
void get_parties(ITEM ***items, int *nb_parties)
{
        ITEM **tab = NULL;
        struct mtp_push_challengers challenger;
        int code,
            i = 0;
        char *label_desc;
        char *label_name;
        
        tab = (ITEM **) malloc(sizeof(ITEM *));
        VERIF_CRITIQUE(tab, NULL, "get_parties: tab malloc fail ");
        
        // On envoi la requete au serveur demande liste labyrinthes
        send_get_challengers(sockfd);
        challenger.more_flag = 0;
        
        do {
                // On lit le code de la prochaine réponse
                code = mtp_get_next_code(sockfd);
                if(code) {
                        // On lit le contenu de la trame, on le met dans labinfo
                        mtp_read(sockfd, &challenger, sizeof(struct mtp_push_challengers));
                        
                        // On créé les textes pour le menu
                        label_desc = (char *)malloc(30 * sizeof(char));
                        label_name = (char *)malloc(40 * sizeof(char));
                        
                        VERIF_CRITIQUE(label_desc, NULL, "get_parties: label_desc malloc fail ");
                        VERIF_CRITIQUE(label_name, NULL, "get_parties: label_name malloc fail ");
                        
                        
                        sprintf(label_name, "%s%s%d", challenger.pseudo, REJOINDRE_FORMAT , challenger.lab_id);
                        sprintf(label_desc, "niveau %d", challenger.niveau);
                        
                        // Attribution tableau <-> (item, func)
                        *(tab + i) = new_item(label_name, label_desc);
                        set_item_userptr(*(tab + i), rejoindre_on_action);
                        i += 1;
                }
                else if(code && code != PUSH_CHALLENGERS) {
                        DEBUG("get_parties: err code incorrect (%d)\n", code);
                        break;
                }
                // On agrandit le tableau
                tab = (ITEM **) realloc(tab, (i + 1) *sizeof(ITEM *));
                VERIF_CRITIQUE(tab, NULL, "get_parties: tab realloc fail ");
        }
        while(challenger.more_flag);
        
        *(tab + i) = NULL;
        
        *nb_parties = i;
        *items = tab;
}

/*******************************************************************************
 **************************** Créer le tableau d'items *************************
 ******************************************************************************/
void get_parties_a_regarder(ITEM ***items, int *nb_parties)
{
        ITEM **tab = NULL;
        struct mtp_push_info_games game;
        int code,
            i = 0;
        char *label_desc;
        char *label_name;
        
        tab = (ITEM **) malloc(sizeof(ITEM *));
        VERIF_CRITIQUE(tab, NULL, "get_parties: tab malloc fail ");
        
        // On envoi la requete au serveur demande liste labyrinthes
        send_get_info_games(sockfd);
        game.more_flag = 0;
        
        do {
                // On lit le code de la prochaine réponse
                code = mtp_get_next_code(sockfd);
                if(code) {
                        // On lit le contenu de la trame, on le met dans labinfo
                        mtp_read(sockfd, &game, sizeof(struct mtp_push_info_games));
                        
                        // On créé les textes pour le menu
                        label_desc = (char *)malloc(30 * sizeof(char));
                        label_name = (char *)malloc(40 * sizeof(char));
                        
                        VERIF_CRITIQUE(label_desc, NULL, "get_parties: label_desc malloc fail ");
                        VERIF_CRITIQUE(label_name, NULL, "get_parties: label_name malloc fail ");
                        
                        
                        sprintf(label_name, "Lab %d - %s et %s", game.lab_id, game.pseudo1, game.pseudo2);
                        sprintf(label_desc, " ");
                        
                        // Attribution tableau <-> (item, func)
                        *(tab + i) = new_item(label_name, label_desc);
                        set_item_userptr(*(tab + i), observer_on_action);
                        i += 1;
                }
                else if(code && code != PUSH_INFO_GAMES) {
                        DEBUG("get_parties: err code incorrect (%d)\n", code);
                        break;
                }
                // On agrandit le tableau
                tab = (ITEM **) realloc(tab, (i + 1) *sizeof(ITEM *));
                VERIF_CRITIQUE(tab, NULL, "get_parties: tab realloc fail ");
        }
        while(game.more_flag);
        
        *(tab + i) = NULL;
        
        *nb_parties = i;
        *items = tab;
}


int     rejoindre(int action, char (*p_pseudo)[PSEUDO_LEN])
{
        ITEM **my_items;
        int c;
        int quit;
        MENU * my_menu;
        int n_choices;
        lab_id = 0;
        pseudoAdversaire = p_pseudo;
        
        /*Initialize items */
        if(action == OBSERVER_PARTIE)
                get_parties_a_regarder(&my_items, &n_choices);
        else
                get_parties(&my_items, &n_choices);
        
        /*Create menu */
        my_menu = new_menu((ITEM **) my_items);
        
        /* INIT */
        forRejoindre = newwin(LINES,COLS, 0, 0);
	keypad(forRejoindre, TRUE);
	set_menu_win(my_menu, forRejoindre);
        
        
        /*Post the menu */
        post_menu(my_menu);
        mvwprintw(forRejoindre, LINES - 2, 0, "F5 pour actualiser, fleche gauche pour retourner au menu principal"); 
        wrefresh(forRejoindre);
        
        if(n_choices == 0) {
                mvwprintw(forRejoindre, 0, 0, "Vide !"); 
        }
        
        quit = 0;
        while (!quit && !lab_id)
        {
                c = wgetch(forRejoindre);
                switch (c)
                {
                        case KEY_DOWN:
                                menu_driver(my_menu, REQ_DOWN_ITEM);
                                break;
                        case KEY_UP:
                                menu_driver(my_menu, REQ_UP_ITEM);
                                break;
                        case 10: {
                                        quit = 1;
                                        ITEM * cur;
                                        void(*p)(char*);
                                        cur = current_item(my_menu);
                                        p = item_userptr(cur);
                                        p((char*) item_name(cur));
                                        pos_menu_cursor(my_menu);
                                        break;
                                }
                                break;
                        case KEY_F(5): 
                                quit = 1;
                                return rejoindre(action, p_pseudo);
                        case KEY_LEFT:
                        case KEY_F(4):
                                quit = 1;
                                break;
                }
                wrefresh(forRejoindre);
        }
        
        unpost_menu(my_menu);
        
        for (c = 0; c < n_choices; ++c) {
                free((char *) item_name(my_items[c]));
                free((char *) item_description(my_items[c]));
                free_item(my_items[c]);
        }
        
        free_menu(my_menu);
        wclear(forRejoindre);
        wrefresh(forRejoindre);
        delwin(forRejoindre);
        
        free((void *)my_items); /* Apres endwin car menu accroché à win ! */
        
        
        DEBUG("Lab choisi : %d\n", lab_id);
        
        return lab_id;
}


