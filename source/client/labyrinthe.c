#include <stdio.h>
#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../constants.h"
#include "../mtp/mtp.h"
#include "../mtp/get.h"
#include "connexion.h"

#define XMAX 25
#define YMAX 80

#define ORIGIN_X  0
#define ORIGIN_Y  0

#define BONHOMME_ADVERSAIRE '!'

#define CHAR_PLAYERS_SAME_PLACE '2'

/* Différence entre 2 struct timespec en CENTISECONDES */
#define TIMESPEC_DIFF(t1, t2) (int)((t2.tv_sec - t1.tv_sec) * 100) + (int)((t2.tv_nsec - t1.tv_nsec) / 1e7)

/* Définies autre part */
char pseudo[PSEUDO_LEN];
extern int sockfd;

/* Labyrinthe */
static char                 **lines;
static struct mtp_push_maze dim;
/* Position */
static struct mtp_get_move  moi;
static int                  nbMove = 0;
static char                 bonhomme_moi;
static struct mtp_push_move adversaire;
static char                 bonhomme_adversaire;


/* Thread related */
static pthread_t       thread_adversaire;
static pthread_t       thread_moi;
static pthread_t       thread_timer;
static pthread_mutex_t window_mutex;
static pthread_mutex_t terminaison_mutex;
static pthread_cond_t  terminaison_cond;

#define TERMINAISON_MOI                 1
#define TERMINAISON_MOI_ABANDON         2
#define TERMINAISON_ADVERSAIRE          3
#define TERMINAISON_ADVERSAIRE_ABANDON  4
static int             terminaison_id;


/* Autres */
WINDOW * window;
static struct timespec heure_debut;
static struct timeval  tv_sauv; /* Sauvegarde opt RECVTIMEO de sockfd */
static struct mtp_push_winner winner;


int handlemovement(int next_y, int next_x);

void confcolors()
{
        start_color();
        init_pair(1, COLOR_YELLOW, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_GREEN);
        init_pair(3, COLOR_RED, COLOR_RED);
}

void load(int id) {
        struct mtp_push_line line;
        code_t code;
        int i, j;
        
        send_get_maze(sockfd, id);
        
        while((code = mtp_get_next_code(sockfd)) != PUSH_MAZE) {
                DEBUG("Mauvais code : %d\n", code);
        }
        
        mtp_read(sockfd, &dim, sizeof(struct mtp_push_maze));
        
	// Mallocs
        lines = NULL;
	lines = (char **) calloc(dim.lignes + 1, sizeof(char *));
	VERIF_CRITIQUE(lines, NULL, "load: calloc fail");
	DEBUG("lines est egal a : %p\n", lines);
	
        // Enregistrement
        for(i = 0; i < dim.lignes; i++) {
                code = mtp_get_next_code(sockfd);
                if(code != PUSH_LINE) {
                        DEBUG("drawmap: mauvais code (%d)recup ligne", code);
                        exit(EXIT_FAILURE);
                }
                mtp_read(sockfd, &line, sizeof(struct mtp_push_line));
                
                lines[i] = (char *) malloc((dim.colonnes + 1) * sizeof(char));
                VERIF_CRITIQUE(lines[i], NULL, "load: malloc fail");
                
                for(j = 0; j < dim.colonnes; j++) {
                        lines[i][j] = line.ligne[j];
                }
                lines[i][j] = 0;
        }
        lines[i] = NULL;
}

int drawmap()
{
        char **lines_tmp = lines;
        char *line;
        char car;
	
        wmove(window, ORIGIN_Y, ORIGIN_X);
	
	for(; *lines_tmp; lines_tmp++) {
	        line = *lines_tmp;
                for(; *line; line++) {
                        car = *line;
                        switch (car) {
                                case ENTREE:
                                        wattron(window, COLOR_PAIR(1));
                                        wprintw(window, "%c", car);
                                        wattroff(window, COLOR_PAIR(1));
                                        break;
                                case MUR_X:
                                case MUR_V:
                                case MUR_H:
                                        wattron(window, COLOR_PAIR(2));
                                        wprintw(window, "%c", car);
                                        wattroff(window, COLOR_PAIR(2));
                                        break;
                                default:
                                        wprintw(window, "%c", car);
                                        break;
                        }
                }
                wprintw(window, "\n");
        }
        return 0;
}

void refreshWin()
{
        VERIF_CRITIQUE_ZERO( pthread_mutex_lock(&window_mutex), "refreshWin: mutex lock echec ");
        
        werase(window);
        confcolors();
        drawmap();
        
        
        if(bonhomme_adversaire && bonhomme_moi && adversaire.coord.y == moi.coord.y && adversaire.coord.x == moi.coord.x) {
                mvwaddch(window, moi.coord.y, moi.coord.x, CHAR_PLAYERS_SAME_PLACE);
        }
        else {
                if(bonhomme_adversaire)
                        mvwaddch(window, adversaire.coord.y, adversaire.coord.x, bonhomme_adversaire);
                if(bonhomme_moi)
                        mvwaddch(window, moi.coord.y, moi.coord.x, bonhomme_moi);
        }
        
        wrefresh(window);
        
        VERIF_CRITIQUE_ZERO( pthread_mutex_unlock(&window_mutex), "refreshWin: mutex unlock echec ");
}

void fini(int term_id) {
        struct timespec fin;
        int temps;

        VERIF_CRITIQUE_ZERO( pthread_mutex_lock(&terminaison_mutex), "fini: terminaison_mutex lock fail");
        terminaison_id = term_id;
        pthread_cond_signal(&terminaison_cond);
        pthread_cancel(thread_adversaire);
        VERIF_CRITIQUE_ZERO( pthread_mutex_unlock(&terminaison_mutex), "fini: terminaison_mutex UNblock fail");
        pthread_cancel(thread_timer);
        VERIF_CRITIQUE( clock_gettime(CLOCK_REALTIME, &fin), -1, "fini: clock fail");
        
        temps = TIMESPEC_DIFF(heure_debut, fin);
        
        if(term_id == TERMINAISON_MOI)
                send_get_winner(sockfd, pseudo, temps);
        else if(term_id == TERMINAISON_MOI_ABANDON)
                send_get_abandon(sockfd);
        
        DEBUG("fini en %d centisec ou %d, %.2f sec!\n", temps, temps/100, temps/100.);
        pthread_exit(pseudo);
}


void inputloop()
{
        int next_y;
        int next_x;
        int keypress;
        
        do {
                send_get_move(sockfd, &moi);
                next_y = moi.coord.y;
                next_x = moi.coord.x;
                nbMove++;
                keypress = wgetch(window);
                switch (keypress) {
                        case ' ':
                        case KEY_F(5):
                                fini(TERMINAISON_MOI_ABANDON);
                                break;
                                
                        case 'q':
                        case KEY_LEFT:
                                next_x--;
                                break;
                        case 's':
                        case KEY_DOWN:
                                next_y++;
                                break;
                        case 'z':
                        case KEY_UP:
                                next_y--;
                                break;
                        case 'd':
                        case KEY_RIGHT:
                                next_x++;
                                break;
                }
                DEBUG("Next x = %d, next y = %d\n", next_x, next_y);
        }
        while(!handlemovement(next_y, next_x));
        
        DEBUG("quit loop\n");
}

int handlemovement(int next_y, int next_x)
{
        DEBUG("HandleMove\n");
        if(   next_y <= ORIGIN_Y || next_y > ORIGIN_Y + dim.lignes - 1
              || next_x <= ORIGIN_X || next_x > ORIGIN_X + dim.colonnes - 1)
                return 0;
        
        char nextchar = mvwinch(window, next_y, next_x) & A_CHARTEXT;
        
        switch (nextchar)
        {
                case ENTREE:
                        if(next_y > ORIGIN_Y + 3) {
                                fini(TERMINAISON_MOI);
                                return 1;
                        }
                        break;
                case MUR_V:
                case MUR_X:
                case MUR_H:
                        break;
                
                default: /* pas "case BLANC:" sinon on peut pas aller sur l'adv. */
                        moi.coord.x = next_x;
                        moi.coord.y = next_y;
                        werase(window);
                        refreshWin();
                        break;
        }
        return 0;
}

void    startgame(void *param) {
        refreshWin();
        VERIF_CRITIQUE(clock_gettime(CLOCK_REALTIME, &heure_debut), -1, "startgame: clock fail");
        inputloop();
}

void    startobserve(void *param) {
        refreshWin();
        VERIF_CRITIQUE(clock_gettime(CLOCK_REALTIME, &heure_debut), -1, "startobserve: clock fail");
        int keypress;
        do {
                keypress = wgetch(window);
        }
        while(keypress != 'q');
        
        DEBUG("quit loop\n");
}

void    disp_time() {
        WINDOW *win_time = newwin(15, 40, 1, dim.colonnes + 2);
        struct timespec wait;
        struct timespec tmp;
        wait.tv_sec = 0;
        wait.tv_nsec = 1<<26;
        pthread_cleanup_push((void *)delwin, (void *)&win_time);
        unsigned int mn = 0;
        unsigned int sec = 0;
        unsigned int csec = 0;
        
        while(1) {
                VERIF_CRITIQUE( clock_gettime(CLOCK_REALTIME, &tmp), -1, "disp_time: clock fail");
                csec = TIMESPEC_DIFF(heure_debut, tmp);
                mn = (int)(csec / 100) / 60;
                sec = (csec / 100) % 60;
                csec %= 100;
                VERIF_CRITIQUE_ZERO( pthread_mutex_lock(&window_mutex), "disp_time: mutex unlock echec ");
                wclear(win_time);
                mvwprintw(win_time, 0, 0, "Durée - %02d:%02d:%02d", mn, sec, csec);
                wrefresh(win_time);
                VERIF_CRITIQUE_ZERO( pthread_mutex_unlock(&window_mutex), "disp_time: mutex unlock echec ");
                nanosleep(&wait, NULL);
        }
        pthread_cleanup_pop(1);
}


void    received_winner() {
        mtp_read(sockfd, &winner, sizeof(struct mtp_push_winner));
        
        VERIF_CRITIQUE_ZERO( pthread_mutex_lock(&terminaison_mutex), "watch_other: terminaison_mutex lock fail");
        
        terminaison_id = TERMINAISON_ADVERSAIRE;
        pthread_cond_signal(&terminaison_cond); 
        pthread_cancel(thread_moi);
        
        VERIF_CRITIQUE_ZERO( pthread_mutex_unlock(&terminaison_mutex), "watch_other: terminaison_mutex UNblock fail");
        
        pthread_cancel(thread_timer);
        pthread_exit(NULL);
}
void    received_abandon() {
        struct mtp_push_abandon abandon;
        
        mtp_read(sockfd, &abandon, sizeof(struct mtp_push_abandon));
        
        VERIF_CRITIQUE_ZERO( pthread_mutex_lock(&terminaison_mutex), "watch_other: terminaison_mutex lock fail");
        
        terminaison_id = TERMINAISON_ADVERSAIRE_ABANDON;
        pthread_cancel(thread_moi);
        pthread_cond_signal(&terminaison_cond); 
        
        VERIF_CRITIQUE_ZERO( pthread_mutex_unlock(&terminaison_mutex), "watch_other: terminaison_mutex UNblock fail");
        
        pthread_cancel(thread_timer);
        pthread_exit(NULL);
}

void    watch_other(void *obj) {
        code_t code;
        struct mtp_push_abandon abandon;
        
        while((code = mtp_get_next_code(sockfd))) {
                switch(code) {
                        case PUSH_MOVE:
                                mtp_read(sockfd, &adversaire, sizeof(struct mtp_push_move));
                                DEBUG("recu un move : %d %d\n", adversaire.coord.x, adversaire.coord.y);
                                refreshWin();
                                break;
                        case PUSH_WINNER: received_winner(); break;
                        case PUSH_ABANDON: received_abandon(); break;
                        default: 
                                DEBUG("CODE BIZARRE %d \n", code);
                }
        }
        
        DEBUG("Serveur déconnecté\n");
        exit(EXIT_FAILURE);
}


void    load_init_position() {
        struct mtp_push_move_extended move_recu;
        code_t code;
        
        code = mtp_get_next_code(sockfd);
        if(code != PUSH_MOVE_EXTENDED)
                DEBUG("load_init_position: bad code player 1");
        
        mtp_read(sockfd, &move_recu, sizeof(struct mtp_push_move_extended));
        
        bonhomme_moi = move_recu.caractere;
        moi.coord.x = move_recu.base.coord.x;
        moi.coord.y = move_recu.base.coord.y;
        
        code = mtp_get_next_code(sockfd);
        if(code != PUSH_MOVE_EXTENDED)
                DEBUG("load_init_position: bad code player 2");
        
        mtp_read(sockfd, &move_recu, sizeof(struct mtp_push_move_extended));
        
        bonhomme_adversaire = move_recu.caractere;
        adversaire.coord.x = move_recu.base.coord.x;
        adversaire.coord.y = move_recu.base.coord.y;
}

void    watch_players(void *obj) {
        code_t code;
        struct mtp_push_move_extended move_recu;
        
        while((code = mtp_get_next_code(sockfd))) {
                switch(code) {
                        case PUSH_MOVE_EXTENDED:
                                mtp_read(sockfd, &move_recu, sizeof(struct mtp_push_move_extended));
                                if(move_recu.caractere == bonhomme_moi) {
                                        moi.coord.x = move_recu.base.coord.x;
                                        moi.coord.y = move_recu.base.coord.y;
                                }
                                else {
                                        adversaire.coord.x = move_recu.base.coord.x;
                                        adversaire.coord.y = move_recu.base.coord.y;
                                }
                                DEBUG("recu un move : %d %d\n", move_recu.base.coord.x, move_recu.base.coord.y);
                                refreshWin();
                                break;
                        case PUSH_WINNER: received_winner(); break;
                        case PUSH_ABANDON: received_abandon(); break;
                        default: 
                                DEBUG("CODE BIZARRE %d \n", code);
                }
        }
        
        DEBUG("Serveur déconnecté\n");
        exit(EXIT_FAILURE);
}

void    wait_other() {
        while( mtp_see_next_code(sockfd) != PUSH_MOVE )
                DEBUG("Attente ...\n");
}

void    fd_expire_infini() {
        struct timeval tv_def;
        socklen_t len_tv_sauv = sizeof(struct timeval);
        memset(&tv_def, 0, sizeof(struct timeval));
        
        VERIF_CRITIQUE( getsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_sauv, &len_tv_sauv),
                        -1, "watch_other: err get old timeval ");
        VERIF_CRITIQUE( setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_def, sizeof(struct timeval)),
                        -1, "watch_other: err set new timeval ");
}

void    afficher_msg_fin(int id) {
        wclear(window);
        
        switch(terminaison_id) {
                case TERMINAISON_MOI:
                        wprintw(window, "J'ai gagné !! :-)");
                        break;
                case TERMINAISON_ADVERSAIRE:
                        wprintw(window, "%s à gagné !! :-(", winner.pseudo);
                        break;
                case TERMINAISON_MOI_ABANDON:
                        wprintw(window, "Courage ...");
                        break;
                case TERMINAISON_ADVERSAIRE_ABANDON:
                        wprintw(window, "Il a abandonné...");
                        break;
        }
        
        wrefresh(window);
        sleep(2);
}

void    labyrinthe_init_ncurse() {
        curs_set(0);
        noecho();
        nonl();
        keypad(window, TRUE);
        keypad(stdscr, TRUE);
}

void    labyrinthe_init_static_vars() {
        lines = NULL;
        thread_adversaire = 0;
        thread_moi = 0;
        thread_timer = 0;
        moi.coord.x = ORIGIN_X + 1;
        moi.coord.y = ORIGIN_Y + 1;
        memset(&adversaire, 0, sizeof(struct mtp_push_move));
        memset(&winner, 0, sizeof(struct mtp_push_winner));
        bonhomme_adversaire = BONHOMME_ADVERSAIRE;
        nbMove = 0;
        terminaison_id = 0;
        window = newwin(72, 72, 0, 0);
        
        VERIF_CRITIQUE_ZERO( pthread_mutex_init(&window_mutex     , NULL), "labyrinthe: mutex window init echec ");
        VERIF_CRITIQUE_ZERO( pthread_mutex_init(&terminaison_mutex, NULL), "labyrinthe: mutex terminaison init echec ");
        VERIF_CRITIQUE_ZERO( pthread_cond_init( &terminaison_cond , NULL), "labyrinthe: mutex terminaison cond init echec ");
}

void    free_labyrinthe_ressources() {

        wclear(window);
        wrefresh(window);
        delwin(window);
        
        /* Liberation ressources */
        VERIF_CRITIQUE_ZERO( pthread_mutex_destroy(&window_mutex),      "labyrinthe: mutex window destroy echec ");
        VERIF_CRITIQUE_ZERO( pthread_mutex_destroy(&terminaison_mutex), "labyrinthe: mutex terminaison destroy echec ");
        VERIF_CRITIQUE_ZERO( pthread_cond_destroy(&terminaison_cond),   "labyrinthe: mutex terminaison cond destroy echec ");
        
	for(int i = 0; lines[i]; i++)
		free(lines[i]);
	
	DEBUG("will free(%p)\n", lines);
	free(lines);
		
	/* Remise à l'état d'origine */
        VERIF_CRITIQUE( setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_sauv, sizeof(struct timeval)),
                        -1, "labyrinthe: err backup timeval ");
        nl();
        curs_set(1);
}

void    attendre_fin_du_jeu() {
        /* En attente de la terminaison : Variable de condition */
        VERIF_CRITIQUE_ZERO( pthread_mutex_lock(&terminaison_mutex), "main: terminaison_mutex lock fail");
        while(!terminaison_id)
                VERIF_CRITIQUE_ZERO( pthread_cond_wait(&terminaison_cond, &terminaison_mutex), "main: wait term fail");
        VERIF_CRITIQUE_ZERO( pthread_mutex_unlock(&terminaison_mutex), "main: terminaison_mutex UNlock fail");
}

void    labyrinthe(int id, char bonhomme)
{
        bonhomme_moi = bonhomme;
        /* Init static var */
        labyrinthe_init_static_vars();
        /* Init ncurse */
        labyrinthe_init_ncurse();
        /* Init game \w server */
        load(id);
        send_get_move(sockfd, &moi);
        fd_expire_infini();
        wprintw(window, "En attente d'adversaire ...");
        wrefresh(window);
        wait_other();
        
        /* Create threads */
        pthread_create(&thread_adversaire, NULL, (void *)watch_other, NULL);
        pthread_create(&thread_moi, NULL, (void *)startgame, NULL);
        pthread_create(&thread_timer, NULL, (void *)disp_time, NULL);
        
        
        attendre_fin_du_jeu();
        
        afficher_msg_fin(terminaison_id);
        
        /* Libération des ressources */
        free_labyrinthe_ressources();
}

void    observer_labyrinthe(int id)
{
        /* Init static var */
        labyrinthe_init_static_vars();
        /* Init ncurse */
        labyrinthe_init_ncurse();
        
        /* Init game \w server */
        load_init_position();
        load(id);
        
        fd_expire_infini();
        
        /* Create threads */
        pthread_create(&thread_adversaire, NULL, (void *)watch_players, NULL);
        pthread_create(&thread_moi, NULL, (void *)startobserve, NULL);
        pthread_create(&thread_timer, NULL, (void *)disp_time, NULL);
        
        attendre_fin_du_jeu();
        
        
        DEBUG("Gagné : %s\n", terminaison_id == TERMINAISON_MOI ? "Moi" : (terminaison_id ? "Adversaire" : "Personne"));
        
        /* Libération des ressources */
        free_labyrinthe_ressources();
}

