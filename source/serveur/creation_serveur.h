#ifndef H_CREATION_SERVEUR
#define H_CREATION_SERVEUR

#include <netinet/in.h>
#include <netdb.h>

void    close_serveur();

/**
 * Résolution du nom de la machine
 */
void    fill_hostent(struct hostent ** host);

/**
 * Création de la sockaddr
 */
void    fill_sockaddr(struct sockaddr_in *sockaddr, int port);

/**
 * Mise en écoute de connexions
 */
void    start();
/**
 * Initialisation du serveur
 */
void    creation_serveur(int port);


#endif


