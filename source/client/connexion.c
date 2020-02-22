#include <ncurses.h>
#include <string.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <menu.h>
#include <unistd.h>

#include "../mtp/mtp.h"
#include "../mtp/get.h"
#include "../constants.h"

#define CONNEXION_LABEL "Veuillez entrer votre pseudo"

extern int sockfd;
extern char pseudo[PSEUDO_LEN];

static WINDOW *forPseudo;

/**
 * @return 0 si CONNU, -1 si INCONNU
 */
int checkPseudo(void)
{
        struct  mtp_push_connect con;
        struct  mtp_push_rec rec;
        code_t  code;
        char    buffer[PSEUDO_LEN];
        int     quitter = 0;
        int     pseudoInvalide = 1;
        
        memset(buffer, 0, PSEUDO_LEN);
        strncpy(buffer, pseudo, PSEUDO_LEN);

        
        send_get_connect(sockfd, pseudo);
        
        while(!quitter) {
                code = mtp_get_next_code(sockfd);
                
                switch (code) {
                        case PUSH_CONNECT:
                                mtp_read(sockfd, &con, sizeof(struct mtp_push_connect));
                                if(con.status == CONNU) { /* Tout est OK */
                                        pseudoInvalide = 0;
                                        quitter = 1;
                                }
                                else if(con.status == INCONNU){
                                        /* Soit pseudo n'existe pas dans BDD 
                                         * Soit quelqu'un est connecté avec celui-ci */
                                        con.message.message[con.message.len - 1] = '\0';
                                        wprintw(forPseudo, "Erreur : %s\n", con.message.message);
                                        pseudoInvalide = 1;
                                        send_get_rec(sockfd, buffer);
                                }
                                else {
                                        DEBUG("checkPseudo: PUSH_CONNECT status inconnu (%d)", con.status);
                                        exit(EXIT_FAILURE);
                                }
                                break;
                        case PUSH_REC:
                                DEBUG("PUSHHHH_REC\n");
                                mtp_read(sockfd, &rec, sizeof(struct mtp_push_rec));
                                if(rec.status == OUI){ /* Pseudo ajouté à la BDD, on peut se connecter avec */
                                        pseudoInvalide = 0;
                                        send_get_connect(sockfd, pseudo);
                                }
                                else if(rec.status == NON){
                                        wprintw(forPseudo, "Veuillez changer de pseudo.\n", con.message.message);
                                        pseudoInvalide = 1;
                                        quitter = 1;
                                }
                                else {
                                        DEBUG("checkPseudo: PUSH_REC status inconnu (%d)", rec.status);
                                        exit(EXIT_FAILURE);
                                }
                                break;
                                
                        default:
                                DEBUG("checkPseudo: Trame reçue incorrecte (%d)\n", code);
                                exit(EXIT_FAILURE);
                }
        }
        return pseudoInvalide;
}

void connexion()
{
        forPseudo = newwin(0,0,0,0);
        echo();
	keypad(forPseudo, TRUE);

        do {
                wprintw(forPseudo,"%s  : ", CONNEXION_LABEL);
                wgetnstr(forPseudo, pseudo, PSEUDO_LEN);
                wclear(forPseudo);
        }
        while(checkPseudo());
        
        noecho();
        delwin(forPseudo);
}


