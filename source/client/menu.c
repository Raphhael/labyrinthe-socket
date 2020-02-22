#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <menu.h>
#include <unistd.h>

#include "../constants.h"
#include "../mtp/get.h"
#include "choix_labyrinthe.h"
#include "historique_parties.h"
#include "choix_bonhomme.h"
#include "rejoindre.h"
#include "menu.h"
#include "labyrinthe.h"

extern int sockfd;

static WINDOW* forMenu;
static ITEM **my_items;

void free_menu_items() {
        free(my_items);
}

void play(int lab_id, char *pseudoEnvoi) {
        DEBUG("Play sur %d\n", lab_id);
        if(!lab_id) return;
        
        char bonhomme = choix_bonhomme();
        
        send_get_challenge(sockfd, pseudoEnvoi, bonhomme, lab_id, 0);
        
        labyrinthe(lab_id, bonhomme);
}

void menu_on_action(char *name)
{
        char pseudo[PSEUDO_LEN];
        int  lab_id;
        
        if (!strcmp(name, MENU_CREATE_PARTY)) {
                play(choix_labyrinthe(), NULL);
        }
        else if (!strcmp(name, MENU_JOIN_PARTY)) {
                lab_id = rejoindre(REJOINDRE_PARTIE, &pseudo);
                DEBUG("REJOINDRE OK SUR %d\n", lab_id);
                
                play(lab_id, pseudo);
        }
        else if (!strcmp(name, MENU_OBSERVE_GAME)) {
                int lab_id = rejoindre(OBSERVER_PARTIE, &pseudo);
                if(lab_id)
                        observer_labyrinthe(lab_id);
        }
        else if (!strcmp(name, MENU_HIST_PARTIE_PSEUDO)) {
                historique_parties(HIST_FROM_PSEUDO);
        }
        else if (!strcmp(name, MENU_HIST_PARTIE_LAB)) {
                historique_parties(HIST_FROM_LAB);
        }
        else if (!strcmp(name, MENU_EXIT)) {
                exit(EXIT_SUCCESS);
        }
        lab_id = 0;
        redrawwin(forMenu);
}

void menu()
{
        DEBUG("Arrivee au Menu\n");
        
        VERIF_CRITIQUE_ZERO(atexit(free_menu_items), "menu: gestionnaire free_menu_items echec");
        
        int c;
        int quit;
        MENU *my_menu;
        int n_choices, i;
        char *choices[] = {
                MENU_CREATE_PARTY,
                MENU_JOIN_PARTY,
                MENU_OBSERVE_GAME,
                MENU_HIST_PARTIE_PSEUDO,
                MENU_HIST_PARTIE_LAB,
                MENU_EXIT,
                NULL
        };
        
        /* Initialize items */
        n_choices = ARRAY_SIZE(choices);
        my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));
        for (i = 0; i < n_choices; ++i) {
                my_items[i] = new_item(choices[i], "");
                set_item_userptr(my_items[i], menu_on_action);
        }
        my_items[n_choices] = (ITEM *)NULL;
        
        /* Create menu */
        my_menu = new_menu((ITEM **)my_items);

	/*window forMenu */
        forMenu = newwin(LINES,COLS, 0, 0);
	keypad(forMenu, TRUE);
	set_menu_win(my_menu, forMenu);
	
        MENU_HELP
        /* Post the menu */
        post_menu(my_menu);
        wrefresh(forMenu);
        
        quit = 0;
        while (!quit) {
                c = wgetch(forMenu);
                switch (c) {
                        case KEY_DOWN:
                                menu_driver(my_menu, REQ_DOWN_ITEM);
                                break;
                        case KEY_UP:
                                menu_driver(my_menu, REQ_UP_ITEM);
                                break;
                        case 10: /* Enter */ {
                                        ITEM *cur;
                                        void (*p)(char *);
                                        cur = current_item(my_menu);
                                        p = item_userptr(cur);
                                        p((char *)item_name(cur));
                                        pos_menu_cursor(my_menu);
                                } break;
                        case KEY_F(1): 
                                quit = 1;
                }
        }
        
        unpost_menu(my_menu);
        for (i = 0; i < n_choices; ++i)
                free_item(my_items[i]);
        free_menu(my_menu);
}

