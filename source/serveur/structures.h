#ifndef H_STRUCTURES
#define H_STRUCTURES

#include <netinet/in.h>
#include "../mtp/mtp.h"


struct liste_elem {
        struct liste      *liste;
        struct liste_elem *next;
        struct liste_elem *prev;
        void * contenu;
};

struct liste {
        size_t elem_len;
        struct liste_elem* first;
        struct liste_elem* last;
};

struct  s_client {
        char                    pseudo[PSEUDO_LEN];
        pthread_t               thread;
        int                     sockfd;
        int                     status;
        struct sockaddr_in      addr;
        struct liste_elem       *partie;
        struct s_coordonnees    coord;
        char                    caractere;
        int                     nb_dep;
};

struct s_partie {
        struct s_client *joueur1;
        struct s_client *joueur2;
        struct liste    observateurs;
        int lab_id;
        pthread_mutex_t winner_mutex;
        pthread_mutex_t join_mutex;
};

#define LISTE_GET(type, elem) ((type *) ((struct liste_elem *) elem)->contenu)

#endif

