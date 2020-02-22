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


/* Le format du champ du menu est : "MENU_PREFIX %d", avec %d = lab_id */
#define MENU_PREFIX "Labyrinthe"

extern int sockfd;
static WINDOW* forChoixLabyrinthe;

static int id_lab = 0;

/** 
 * Lorsque l'utilisateur presse <Enter>
 * @var name   Le nom du champ séléctionné
 */
void on_lab_choice(char *name) {
        if (!strncmp(MENU_PREFIX, name, strlen(MENU_PREFIX))) {
                sscanf(name, "Labyrinthe %d", &id_lab);
                send_get_selected_maze(sockfd, id_lab);
        }
}


/*******************************************************************************
 ******* Créer le tableau d'items et charger les labyrinthes de la BDD *********
 ******************************************************************************/
void get_labyrinthes(ITEM ***items, int *nb_labyrinthes)
{
        ITEM **tab = NULL;
        struct mtp_push_info_mazes labinfo;
        int code, 
            i = 0;
        char *label_desc;
        char *label_name;
        
        tab = (ITEM **) malloc(sizeof(ITEM *));
        VERIF_CRITIQUE(tab, NULL, "get_labyrinthes: tab malloc fail ");
        
        // On envoi la requete au serveur demande liste labyrinthes
        send_get_info_mazes(sockfd);
        
        do {
                // On lit le code de la prochaine réponse
                code = mtp_get_next_code(sockfd);
                if(code != PUSH_INFO_MAZES) {
                        DEBUG("get_labyrinthes: err code incorrect (%d)\n", code);
                        break;
                }
                // On lit le contenu de la trame, on le met dans labinfo
                mtp_read(sockfd, &labinfo, sizeof(struct mtp_push_info_mazes));
                
                // On créé les textes pour le menu
                label_desc = (char *)malloc(20 * sizeof(char));
                label_name = (char *)malloc(20 * sizeof(char));

                VERIF_CRITIQUE(label_desc, NULL, "get_labyrinthes: label_desc malloc fail ");
                VERIF_CRITIQUE(label_name, NULL, "get_labyrinthes: label_name malloc fail ");
                
                sprintf(label_name, "%s %d", MENU_PREFIX, labinfo.lab_id);
                sprintf(label_desc, "Niveau %d", labinfo.niveau);
                
                // Attribution tableau <-> (item, func)
                *(tab + i) = new_item(label_name, label_desc);
                set_item_userptr(*(tab + i), on_lab_choice);
                
                // On agrandit le tableau
                tab = (ITEM **) realloc(tab, (i + 2) *sizeof(ITEM *));
                VERIF_CRITIQUE(tab, NULL, "get_labyrinthes: tab realloc fail ");
                
                i++;
        }
        while(labinfo.more_flag);
        
        // On rajoute l'option retour
        *(tab + i) = new_item(MENU_RETOUR, "");
        set_item_userptr(*(tab + i), on_lab_choice);
        *(tab + i + 1) = NULL;

        *nb_labyrinthes = i - 1;
        *items = tab;
}

/*******************************************************************************
 ************** Affichage de l'écran choix d'un labyrinthe *********************
 ******************************************************************************/
int     choix_labyrinthe(void)
{
        ITEM **my_items = NULL;
        MENU *my_menu = NULL;
        ITEM *cur_item = NULL;
        int c;
        int nb_labyrinthes;
        int quit;
        id_lab = 0;

        /*Initialize items */
        get_labyrinthes(&my_items, &nb_labyrinthes);

        /*Create menu */
        my_menu = new_menu(my_items);
        
        
        forChoixLabyrinthe = newwin(LINES,COLS,0, 0);
	keypad(forChoixLabyrinthe, TRUE);
	set_menu_win(my_menu, forChoixLabyrinthe);
        
        MENU_HELP
        /*Post the menu */
        post_menu(my_menu);
        wrefresh(forChoixLabyrinthe);
        
        quit = 0;
        while (!quit && !id_lab){
                c = wgetch(forChoixLabyrinthe);
                switch (c) {
                        case KEY_DOWN:
                                menu_driver(my_menu, REQ_DOWN_ITEM);
                                break;
                        case KEY_UP:
                                menu_driver(my_menu, REQ_UP_ITEM);
                                break;
                        case 10: { /*Enter */
                                        quit = 1;
                                        void(*p)(char*);
                                        cur_item = current_item(my_menu);
                                        p = item_userptr(cur_item);
                                        p((char*) item_name(cur_item));
                                        pos_menu_cursor(my_menu);
                                        break;
                                }
                        case KEY_F(1):
                                quit = 1;
                }
                wrefresh(forChoixLabyrinthe);
        }
        
        unpost_menu(my_menu);
        
        for (c = 0; c <= nb_labyrinthes; ++c) {
                free((char *) item_name(my_items[c]));
                free((char *) item_description(my_items[c]));
                free_item((my_items)[c]);
        }
        
        free(my_items);
        free_menu(my_menu);
        
        werase(forChoixLabyrinthe);
        wrefresh(forChoixLabyrinthe);
        delwin(forChoixLabyrinthe);
        
        return id_lab;
}


