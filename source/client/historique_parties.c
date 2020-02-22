#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <menu.h>
#include <arpa/inet.h>

#include "../constants.h"
#include "../mtp/mtp.h"
#include "../mtp/get.h"

#define HIST_MENU_QUIT     1
#define HIST_MENU_WRITE    2
#define HIST_HEADER_HEIGHT 3

extern int sockfd;

static char saisie[PSEUDO_LEN];
static int  saisieEntier;
static WINDOW *w_center;
static WINDOW *w_header;
static int parent_x, parent_y, searchFrom;

/*******************************************************************************
 *********** Créer le tableau d'items et charger l'hist dans la BDD ************
 ******************************************************************************/
void get_hist_parties(ITEM ***items, int *nb_parties)
{
        ITEM **tab = NULL;
        struct mtp_push_hist histinfo;
        int code,
            i = 0;
        char *label_desc;
        char *label_name;
        
        tab = (ITEM **) malloc(sizeof(ITEM *));
        VERIF_CRITIQUE(tab, NULL, "get_hist_parties: tab malloc fail ");
        
        // On envoi la requete au serveur demande liste labyrinthes
        if(searchFrom == HIST_FROM_LAB)
                send_get_hist_maze(sockfd, saisieEntier);
        else
                send_get_hist_player(sockfd, saisie);
        
        histinfo.more_flag = 0;
        
        do {
                // On lit le code de la prochaine réponse
                code = mtp_get_next_code(sockfd);
                if(code) {
                        // On lit le contenu de la trame, on le met dans labinfo
                        mtp_read(sockfd, &histinfo, sizeof(struct mtp_push_hist));
                        
                        // On créé les textes pour le menu
                        label_desc = (char *)malloc(30 * sizeof(char));
                        label_name = (char *)malloc(40 * sizeof(char));
                        
                        VERIF_CRITIQUE(label_desc, NULL, "get_hist_parties: label_desc malloc fail ");
                        VERIF_CRITIQUE(label_name, NULL, "get_hist_parties: label_name malloc fail ");
                        
                        sprintf(label_name, "Partie %d", histinfo.partie_id);
                        sprintf(label_desc, "%d deplacements en %.2fs", 
                                        histinfo.nombre_dep, ntohs(histinfo.duree)/100.);
                        
                        // Attribution tableau <-> (item, func)
                        *(tab + i) = new_item(label_name, label_desc);
                        i += 1;
                }
                else if(code && code != PUSH_HIST) {
                        DEBUG("get_hist_parties: err code incorrect (%d)\n", code);
                        break;
                }
                // On agrandit le tableau
                tab = (ITEM **) realloc(tab, (i + 1) *sizeof(ITEM *));
                VERIF_CRITIQUE(tab, NULL, "get_hist_parties: tab realloc fail ");
        }
        while(histinfo.more_flag);
        
        *(tab + i) = NULL;
        
        *nb_parties = i;
        *items = tab;
}

int afficher_liste()
{

        int quit = 0, c;
        ITEM **my_items;
        MENU * my_menu;
        int n_choices;

        get_hist_parties(&my_items, &n_choices);
        /*Create menu */
        my_menu = new_menu((ITEM **) my_items);
        set_menu_win(my_menu, w_center);
        set_menu_sub(my_menu, derwin(w_center, parent_y - 8, 38, 3, 1));
        set_menu_mark(my_menu, " * ");
        set_menu_format(my_menu, 10, 1);
        box(w_center, 0, 0);
        post_menu(my_menu);

        do {
                wrefresh(w_center);
                c = wgetch(w_center);

                switch (c)
                {
                        case KEY_DOWN:
                                menu_driver(my_menu, REQ_DOWN_ITEM);
                                break;
                        case KEY_UP:
                                menu_driver(my_menu, REQ_UP_ITEM);
                                break;
                        case KEY_BACKSPACE:
                                quit = HIST_MENU_WRITE;
                                menu_driver(my_menu, REQ_SCR_DPAGE);
                                break;
                        case KEY_LEFT:
                        case 'q':
                                quit = HIST_MENU_QUIT;
                                break;
                        case KEY_NPAGE:
                                menu_driver(my_menu, REQ_SCR_DPAGE);
                                break;
                        case KEY_PPAGE:
                                menu_driver(my_menu, REQ_SCR_UPAGE);
                                break;
                }
        }
        while (!quit);
        
        unpost_menu(my_menu);
        
        for (c = 0; c < n_choices; ++c) {
                free((char *) item_name(my_items[c]));
                free((char *) item_description(my_items[c]));
                free_item(my_items[c]);
        }
        
        free_menu(my_menu);
        endwin();
        
        free((void *)my_items); /* Apres endwin car menu accroché à win ! */
        
        return quit;
}

void hist_init() {
        getmaxyx(stdscr, parent_y, parent_x);

        w_center = newwin(parent_y - HIST_HEADER_HEIGHT, parent_x, HIST_HEADER_HEIGHT, 0);
        w_header = newwin(HIST_HEADER_HEIGHT, parent_x, 0, 0);

        keypad(w_header, TRUE);
        keypad(w_center, TRUE);
}

void hist_end() {
        curs_set(1);
        noecho();
}

void historique_parties(int searchFromWhat)
{
        int quit = 0;
        searchFrom = searchFromWhat;
        hist_init();
        do {
                wclear(w_header);
                wclear(w_center);
                box(w_header, 0, 0);
                
                mvwprintw(w_header, 1, 1, searchFromWhat == HIST_FROM_PSEUDO ?
                                                "pseudo : " : "id de lab : ");

                curs_set(1);
                echo();
                
                wrefresh(w_center);
                wrefresh(w_header);
                wgetnstr(w_header, saisie, PSEUDO_LEN);
                
                
                
                if(searchFromWhat == HIST_FROM_LAB) {
                        saisieEntier = atoi(saisie);
                }
                
                curs_set(0);
                noecho();
                
                quit = afficher_liste();
        }
        while (quit != HIST_MENU_QUIT);
        hist_end();
}



