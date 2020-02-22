#include <curses.h>
#include <menu.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../constants.h"
#include "../mtp/mtp.h"
#include "../mtp/get.h"

#define BONHOMMES        "@+-#&~^=ABCZ" //liste des choix

/*
 * @return Le caractère choisi
 */
char choix_bonhomme(void)
{
        DEBUG("Choisir bonhomme\n");
        WINDOW * window;
        char mon_bonhomme;
        char liste_bohommes[] = BONHOMMES;

        /*Initialize curses 
        initscr();
        start_color();
        cbreak();
        noecho();
        //keypad(stdscr, TRUE);
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_CYAN, COLOR_BLACK);*/
        //erase();
        window = newwin(20, 60, 5, 4);
        keypad(window, TRUE);

        wprintw(window, "Veuillez choisir votre bonhomme");
        int nbChar = strlen(liste_bohommes);
        
       	//afficher les carateres accepter
        for (int i = 0; i < nbChar; i++) {
                if (i % 4 == 0)
                        wprintw(window, "\n");
                wprintw(window, "%c\t", liste_bohommes[i]);
        }

        wrefresh(window);

        do {
                mon_bonhomme = wgetch(window);
        }
        /* strchr renvoie NULL si mon_bonhomme n'existe pas dans liste_bohommes */
        while(!strchr(liste_bohommes, mon_bonhomme));
        
        werase(window);
        wrefresh(window);
        delwin(window);
        
        DEBUG("Bonhomme choisi : %c\n", mon_bonhomme);
        return mon_bonhomme;
}


