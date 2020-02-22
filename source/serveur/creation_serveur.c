#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>

#ifndef __USE_POSIX
        #define __USE_POSIX
#endif
#include <limits.h> /* HOST_NAME_MAX */

#include "structures.h"
#include "liste_chainee.h"
#include "gestion_connexions.h"

#define ACCEPT_QUEUE_LEN         5

extern struct  liste  clients;

static struct  sockaddr_in  addr;
struct s_client *client;
static int      sockfd;

void    close_serveur() {
        if(sockfd)
                close(sockfd);
        liste_destroy(&clients, 1);
        if(client)
                free(client);
}

/**
 * Résolution du nom de la machine
 */
void    fill_hostent(struct hostent ** host) {
        char nom[HOST_NAME_MAX+1];
        
        VERIF_CRITIQUE(gethostname(nom, HOST_NAME_MAX), -1,
                       "fill_sockaddr: Impossible de trouver le nom de la machine. ");
        
        *host = gethostbyname(nom);
        VERIF_CRITIQUE(host, NULL, "fill_sockaddr: Impossible de récupérer la structure hostent du serveur. ");
}

/**
 * Création de la sockaddr
 */
void    fill_sockaddr(struct sockaddr_in *sockaddr, int port) {
        struct hostent * s_host;
        
        fill_hostent(&s_host);
        
        memset((struct sockaddr_in *) sockaddr, 0, sizeof(struct sockaddr_in));
        
        sockaddr->sin_port = htons(port);
        sockaddr->sin_family = AF_INET;
        /*memcpy(
                (struct in_addr *)& sockaddr->sin_addr,
                (char *) s_host->h_addr,
                sizeof(struct in_addr)
        );*/
        
        sockaddr->sin_addr.s_addr = INADDR_ANY;
        
        printf("IP   Serveur : %s\n", inet_ntoa(sockaddr->sin_addr));
        printf("Port Serveur : %hd\n", ntohs(sockaddr->sin_port));
}

/**
 * Mise en écoute de connexions
 */
void    start() {
        socklen_t sockaddrlen;
        struct liste_elem *client_elem;
        client = NULL;
        
        sockaddrlen = sizeof(struct sockaddr_in);
        
        while(1) {
                if(!client) {
                        client = (struct s_client *)malloc(sizeof(struct s_client));
                        VERIF_CRITIQUE(client, NULL, "start: plus de mémoire allocation client");
                        memset((struct s_client *) client, 0, sizeof(struct s_client));
                }
                
                memset((struct sockaddr_in *)&client->addr, 0, sizeof(struct sockaddr_in));
                
                client->sockfd = accept(sockfd, (struct sockaddr *)&client->addr, &sockaddrlen);
                
                if(client->sockfd == -1) {
                        perror("start: echec accept() ");
                        sleep(1);
                }
                else {
                        if(sockaddrlen != sizeof(struct sockaddr_in)){
                                fprintf(stderr, "start: addrlen est passé de %d à %d\n",
                                        sizeof(struct sockaddr_in),
                                        sockaddrlen);
                                sockaddrlen = sizeof(struct sockaddr_in);
                        }
                        
                        client_elem = liste_insert((struct s_client *) client, &clients);
                        
                        if(pthread_create(&client->thread, NULL, (void *)communiquer, client_elem) ) {
                                perror("start: création du thread à échoué ");
                        }
                        
                        client = NULL;
                }
        }
}

/**
 * Initialisation du serveur
 */
void    creation_serveur(int port) {
        if(atexit(close_serveur))
                fprintf(stderr, "Impossible d'installer le gestionnaire de fin \"close_serveur\".\n");
        
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	VERIF_CRITIQUE(sockfd, -1, "create_socket: Impossible de créer la socket");
	
	fill_sockaddr(&addr, port);
	
	VERIF_CRITIQUE(
	        bind(sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)),
	        -1, "create_socket: echec du nommage de la socket. "
        );
        
        VERIF_CRITIQUE( listen(sockfd, ACCEPT_QUEUE_LEN), -1, "create_socket: echec listen ");
}

