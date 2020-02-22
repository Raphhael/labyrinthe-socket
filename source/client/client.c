#include <stdlib.h>
#include <stdio.h>
#include <netdb.h> /* dns */
#include <netinet/ip.h>
#include <arpa/inet.h> /* inet_ntoa, htons */
#include <sys/socket.h>
#include <signal.h>
#include <ncurses.h>
#include <sys/types.h>
#include <string.h> /* mem--- */
#include <unistd.h> /* write */

#ifndef DEBUG_ON
        #include <mcheck.h>
#endif

#include "../constants.h"
#include "../mtp/mtp.h"
#include "../mtp/get.h"
#include "connexion.h"
#include "menu.h"

struct sockaddr_in server;
int sockfd;

/* Chercher le server et remplir la sockaddr */
void    init_server(const char *serveur_name, int port) {
        struct hostent *serverinfo = NULL;
        /* contacter le DNS */
        serverinfo = gethostbyname(serveur_name);
        VERIF_CRITIQUE(serverinfo, NULL, "init_server: echec du DNS ");
        /* reset la sockaddr */
        memset((struct sockaddr_in *) &server, 0, sizeof(struct sockaddr_in));
        /* remplir la sockaddr */
        server.sin_port = htons(port);
        server.sin_family = serverinfo->h_addrtype;
        memcpy( (struct in_addr *) &server.sin_addr,
                (char *) serverinfo->h_addr,
                sizeof(struct in_addr));
        
        printf("%s est à l'adresse %s\n", serveur_name, inet_ntoa(server.sin_addr));
}

/* Création de la socket */
void    socket_open(void) {
        struct timeval maxtime;
        
        maxtime.tv_sec = 1;
        maxtime.tv_usec = 0;
        
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        VERIF_CRITIQUE(sockfd, -1, "socket_open: Impossible d'ouvrir la socket ");
        
        
        VERIF_CRITIQUE( setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &maxtime, sizeof(struct timeval)),
                        -1, "socket_open: erreur temps max reception ");
}

void    on_sortie() {
        endwin();
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
        
        if(argc < 3) {
                DEBUG("Syntaxe : %s <nom serveur> <port>\n", argv[0]);
                return EXIT_FAILURE;
        }
        
        sigint_to_exit();
        
        socket_open();
        init_server(argv[1], atoi(argv[2]));
        
        VERIF_CRITIQUE(connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)),
           -1, "main: erreur connect ");
        // connecté
        
        
        if(atexit(on_sortie))
                DEBUG("Impossible d'installer le gestionnaire de fin \"on_sortie\".\n");
        
        /* INIT NCURSE */
        initscr();
        start_color();
        cbreak();
        noecho();
        //keypad(stdscr, TRUE);
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
        
        connexion();
        // authentifié
        menu();
        
        return EXIT_SUCCESS;
}
