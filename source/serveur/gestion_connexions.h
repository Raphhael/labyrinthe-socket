#ifndef H_GESTION_CONNEXIONS
#define H_GESTION_CONNEXIONS

#include "../constants.h"
#include "structures.h"

void    envoyer_challenger(void * sockfd, void *v_partie, int is_last);

struct s_client *trouverClient(char pseudo[PSEUDO_LEN]);

void    connexion(struct liste_elem *client);

void    communiquer(void *void_cli);

void    init_connexions();

#endif

