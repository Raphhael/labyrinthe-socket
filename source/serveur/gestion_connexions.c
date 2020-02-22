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

#include "../constants.h"
#include "../mtp/mtp.h"
#include "../mtp/push.h"
#include "../bdd/serveur.h"
#include "../bdd/util.h"
#include "liste_chainee.h"
#include "structures.h"

#define MAX_TENTATIVES_CONNEXION 5

#define STATUS_EN_OBSERVATION   1
#define STATUS_DEFAUT           0

#define GET_PARTIE(client) LISTE_GET(struct s_partie, client->partie)
#define GET_ADVERSAIRE(client) GET_PARTIE(client)->joueur1 == client ? GET_PARTIE(client)->joueur2 : GET_PARTIE(client)->joueur1
#define TROUVER_CLIENT(pseudo) ((struct s_client *) liste_rechercher_obj(pseudo, 0, PSEUDO_LEN, &clients))

extern struct  liste  clients;

static struct  liste  parties;

static pthread_mutex_t connexion_mutex;

void    gestion_connexions_sortie() {
        liste_destroy(&parties, 1);
        VERIF_CRITIQUE_ZERO(pthread_mutex_destroy(&connexion_mutex), "on_sortie: connexion_mutex destroy");
}

void    envoyer_parties_en_cours(void * sockfd, void *v_partie, int is_last) {
        struct s_partie *partie = (struct s_partie *) v_partie;
        
        if(partie->joueur1 && partie->joueur2)
                send_push_info_games(*(int *)sockfd,
                                     partie->joueur1->pseudo, 
                                     partie->joueur2->pseudo,
                                     partie->lab_id, is_last ? 0 : 1);
}

void    propager_deplacement_observateurs(void * v_clientQuiABouge, void *v_observateur, int is_last) {
        struct s_client *clientQuiABouge = (struct s_client *) v_clientQuiABouge;
        struct s_client *observateur = (struct s_client *) v_observateur;
        
        send_push_move_extended(observateur->sockfd, clientQuiABouge->coord.x,
                                clientQuiABouge->coord.y, clientQuiABouge->caractere);
        
}

void    propager_abandon_observateurs(void * inutile, void *v_observateur, int is_last) {
        struct s_client *observateur = (struct s_client *) v_observateur;
        
        send_push_abandon(observateur->sockfd);
        
        observateur->status = STATUS_DEFAUT;
        observateur->partie = NULL;
}


void    propager_vainqueur_observateurs(void * b_pseudo, void *v_observateur, int is_last) {
        struct s_client *observateur = (struct s_client *) v_observateur;
        
        send_push_winner(observateur->sockfd, (const char *)b_pseudo);
        
        observateur->status = STATUS_DEFAUT;
        observateur->partie = NULL;
}

void    envoyer_challenger(void * sockfd, void *v_partie, int is_last) {
        struct s_partie *partie = (struct s_partie *) v_partie;
        struct s_client *owner;
        
        if(partie->joueur1 && partie->joueur2)
                return;
        
        owner = partie->joueur1 ? partie->joueur1 : partie->joueur2;
        
        send_push_challengers(*(int *)sockfd,
                              owner->pseudo,
                              partie->lab_id,
                              get_lab_niveau(partie->lab_id), is_last ? 0 : 1);
}

void    free_client(struct liste_elem *e_client) {
        close(LISTE_GET(struct s_client, e_client)->sockfd);
        free((struct s_client *) liste_supprimer(e_client));
}

void    free_partie(struct liste_elem *el_partie) {
        struct s_partie *partie = LISTE_GET(struct s_partie, el_partie);
        printf("free_partie enter\n");
        VERIF_CRITIQUE_ZERO(pthread_mutex_destroy(&partie->winner_mutex), "free_partie: winner_mutex destroy");
        VERIF_CRITIQUE_ZERO(pthread_mutex_destroy(&partie->join_mutex), "free_partie: join_mutex destroy");
        
        printf("free_partie destroy mutex done\n");
        if(partie->joueur1) {
                partie->joueur1->partie = NULL;
                partie->joueur1->nb_dep = 0;
        }
        if(partie->joueur2) {
                partie->joueur2->partie = NULL;
                partie->joueur2->nb_dep = 0;
        }
        
        printf("free_partie will destroy obs\n");
        liste_destroy(&partie->observateurs, 0);
        
        printf("free_partie will free and suppr el_partie\n");
        free((struct s_partie *)liste_supprimer(el_partie));
        printf("free_partie el_partie done\n");
}


void    connexion(struct liste_elem *cli_elem) {
        struct s_client * client = LISTE_GET(struct s_client, cli_elem);
        struct mtp_get_connect mgc;
        struct mtp_get_rec     mgr;
        
        int  echec = 1;
        int  code;
        int  cb_status;
        int  nb_tentatives = 0;
        char txt_response[MESSAGE_LEN];
        
        
        while(echec) {
                code = mtp_get_next_code(client->sockfd);
                
                if(!code || nb_tentatives >= MAX_TENTATIVES_CONNEXION) {
                        send_push_disconnect(client->sockfd, "Nombre de tentatives de connexion trop élevé");
                        free_client(cli_elem);
                        pthread_exit(NULL);
                }
                /* Scénario à éviter :
                 * Thread_J1 reçois demande de connexion pour "user"
                 *   trouverClient("user") faux
                 *   check_pseudo("user") vrai
                 * Thread_J2 reçois demande de connexion pour "user"
                 *   trouverClient("user") faux
                 *   check_pseudo("user") vrai
                 * Thread_J1 enregistre "user" dans la liste des clients connectés
                 * Thread_J2 enregistre "user" dans la liste des clients connectés
                 * ...
                 */
                if(code == GET_CONNECT) { /* Demande de connexion */
                        mtp_read(client->sockfd, &mgc, sizeof(struct mtp_get_connect));
                        nb_tentatives++;
                        
                        VERIF_CRITIQUE_ZERO(pthread_mutex_lock(&connexion_mutex), "connexion: connexion_mutex lock ");
                        if(TROUVER_CLIENT(mgc.pseudo)) {
                                /* Le pseudo existe dans la liste des clients connectés */
                                memset(txt_response, 0, MESSAGE_LEN);
                                sprintf(txt_response, "Pseudo utilisé : \"%s\"", mgc.pseudo);
                                cb_status = INCONNU;
                        }
                        else if(check_pseudo(mgc.pseudo)) {
                                /* Le pseudo existe dans la BDD -> OK */
                                strncpy(client->pseudo, mgc.pseudo, PSEUDO_LEN);
                                cb_status = CONNU;
                                txt_response[0] = '\0';
                                echec = 0;
                        }
                        else { /* Pseudo inexistant dans BDD et dispo */
                                sprintf(txt_response, "Le pseudo \"%s\" est inexistant", mgc.pseudo);
                                cb_status = INCONNU;
                        }
                        VERIF_CRITIQUE_ZERO(pthread_mutex_unlock(&connexion_mutex), "connexion: connexion_mutex UNlock ");
                        send_push_connect(client->sockfd, cb_status, txt_response);
                }
                else if(code == GET_REC) { /* Demande d'enregistrement de pseudo */
                        mtp_read(client->sockfd, &mgr, sizeof(struct mtp_get_rec));

                        if(TROUVER_CLIENT(mgr.pseudo))
                                /* Déjà existant dans la liste des clients connectés */
                                send_push_rec(client->sockfd, NON);
                        else {
                                if(!check_pseudo(mgc.pseudo)) {
                                        /* Le pseudo n'existe pas dans la BDD -> On ajoute */
                                        ajouter_pseudo(mgr.pseudo);
                                        send_push_rec(client->sockfd, OUI);
                                }
                                else  /* Sinon ça va pas */
                                        send_push_rec(client->sockfd, NON);
                        }
                }
                else {
                        strcpy(txt_response, "Vous devez vous connecter d'abord.");
                        send_push_connect(client->sockfd, INCONNU, txt_response);
                }
        }
        printf("%s: Connecté en tant que : %s (@%p)\n", inet_ntoa(client->addr.sin_addr), mgc.pseudo, client);
}

void    init_connexions() {
        if(atexit(gestion_connexions_sortie))
                fprintf(stderr, "Impossible d'installer le gestionnaire de fin \"gestion_connexions_sortie\".\n");
        liste_init(sizeof(struct s_partie), &parties);
        
        VERIF_CRITIQUE_ZERO(pthread_mutex_init(&connexion_mutex, NULL), "main: connexion_mutex init");
}


/**
 * Supprime un client des parties en cours
 * @var cli_elem        paramètre du thread
 */
void    remove_client_from_game(void *cli_elem) {
        struct s_client *client = LISTE_GET(struct s_client, cli_elem);
        int needFreePartie = 0;
        
        if(client->partie) {
                printf("%s était en train de jouer et veut partir !\n", client->pseudo);
                if(client->status == STATUS_EN_OBSERVATION) {
                        /* Le client était en train d'observer une partie */
                        liste_suppr_obj(client, &GET_PARTIE(client)->observateurs);
                }
                else {  /* Était en train de jouer */
                
                        VERIF_CRITIQUE_ZERO(pthread_mutex_lock(&GET_PARTIE(client)->join_mutex), "GET_JOIN: join_mutex lock ");
                        VERIF_CRITIQUE_ZERO(pthread_mutex_lock(&GET_PARTIE(client)->winner_mutex), "GET_JOIN: winner lock ");
                        
                        if(GET_PARTIE(client)->joueur1 == client) {
                                GET_PARTIE(client)->joueur1 = GET_PARTIE(client)->joueur2;
                        }
                        
                        GET_PARTIE(client)->joueur2 = NULL;
                        
                        
                        if(!GET_PARTIE(client)->joueur1) {
                                /* Si plus personne dans la partie
                                 * il peut quand meme rester des obs. arrivés
                                 * entre J1 et J2 */
                                liste_foreach(&GET_PARTIE(client)->observateurs, propager_abandon_observateurs, NULL);
                                needFreePartie = 1;
                        }
                        else if(GET_PARTIE(client)->joueur1->nb_dep) {
                                /* Si la partie avait déjà commencé, c'est un abandon */
                                liste_foreach(&GET_PARTIE(client)->observateurs, propager_abandon_observateurs, NULL);
                                send_push_abandon(GET_PARTIE(client)->joueur1->sockfd);
                                GET_PARTIE(client)->joueur1->partie = NULL;
                                needFreePartie = 1;
                        }
                        /* Sinon il reste un joueur en train de choisir son joueur*/
                        VERIF_CRITIQUE_ZERO(pthread_mutex_unlock(&GET_PARTIE(client)->join_mutex), "GET_JOIN: join_mutex lock ");
                        VERIF_CRITIQUE_ZERO(pthread_mutex_unlock(&GET_PARTIE(client)->winner_mutex), "GET_JOIN: winner lock ");
                        
                        if(needFreePartie)
                                free_partie(client->partie);
                }
        }
}

void    create_partie(int lab, struct s_client *client) {
        struct s_partie *partie = NULL;
        
        partie = (struct s_partie *)malloc(sizeof(struct s_partie));
        VERIF_CRITIQUE(partie, NULL, "create_partie: malloc echec ");
        
        memset(partie, 0, sizeof(struct s_partie));
        
        client->partie = liste_insert((struct s_partie *) partie, &parties);
        
        liste_init(sizeof(struct s_client), &partie->observateurs);
        
        partie->joueur1 = client;
        partie->lab_id = lab;
        
        VERIF_CRITIQUE_ZERO(pthread_mutex_init(&partie->winner_mutex, NULL), "GET_SELECTED_MAZE: winner_mutex init");
        VERIF_CRITIQUE_ZERO(pthread_mutex_init(&partie->join_mutex, NULL), "GET_SELECTED_MAZE: join_mutex init");
        
        printf("%s: Créé partie sur le labyrinthe %d\n", partie->joueur1->pseudo, partie->lab_id);

}

void    communiquer(void *cli_elem) {
        struct s_client * client = LISTE_GET(struct s_client, cli_elem);
        //struct s_partie * partie_client;
        code_t code;
        
        connexion((struct liste_elem *)cli_elem);
        
        do {
                code = mtp_get_next_code(client->sockfd);
                //printf("%s: Recu code %d\n", client->pseudo, code);
                switch(code) {
                        case GET_INFO_MAZES: {
                                        struct mtp_get_info_mazes  gim;
                                        mtp_read(client->sockfd, &gim, sizeof(struct mtp_get_info_mazes));
                                        
                                        get_liste_labyrinthes(client->sockfd);
                                } break;
                                
                        case GET_HIST_PLAYER: {
                                        struct mtp_get_hist_player ghp;
                                        mtp_read(client->sockfd, &ghp, sizeof(struct mtp_get_hist_player));
                                        
                                        get_historique_pseudo(client->sockfd, ghp.pseudo);
                                } break;
                        case GET_HIST_MAZE: {
                                        struct mtp_get_hist_maze ghm;
                                        mtp_read(client->sockfd, &ghm, sizeof(struct mtp_get_hist_maze));
                                        
                                        get_historique_labyrinthe(client->sockfd, ghm.lab_id);
                                } break;
                        
                        case GET_SELECTED_MAZE: {
                                        struct mtp_get_selected_maze  sel;
                                        mtp_read(client->sockfd, &sel, sizeof(struct mtp_get_selected_maze));
                                        create_partie(sel.lab_id, client);
                                } break;
                        case GET_INFO_GAMES: {
                                        struct mtp_get_info_games gc;
                                        mtp_read(client->sockfd, &gc, sizeof(struct mtp_get_info_games));
                                        
                                        printf("%s: Demande des parties en cours\n", client->pseudo);
                                        liste_foreach(&parties, envoyer_parties_en_cours, &client->sockfd);
                                } break;
                                
                        case GET_CHALLENGERS: {
                                        struct mtp_get_challengers gc;
                                        mtp_read(client->sockfd, &gc, sizeof(struct mtp_get_challengers));
                                        
                                        printf("%s: Demande des parties en attentes\n", client->pseudo);
                                        liste_foreach(&parties, envoyer_challenger, &client->sockfd);
                                } break;
                                
                        /* 
                         *  Scénario à éviter :
                         * J1 join partie de J3
                         * J2 join partie de J3 
                                quand Thread_J1 a déjà vérifié que la partie
                                était dispo mais n'a pas encore enregistré que
                                J1 est dans partie de J3
                         */
                        case GET_JOIN: {
                                        struct mtp_get_join mgj;
                                        struct s_client *autrejoueur;
                                        mtp_read(client->sockfd, &mgj, sizeof(struct mtp_get_join));
                                        
                                        printf("%s: Rejoint la partie de %s ... ", client->pseudo, mgj.pseudo);
                                        autrejoueur = (struct s_client *) liste_rechercher_obj(mgj.pseudo, 0, PSEUDO_LEN, &clients);
                                                                
                                        if(!autrejoueur) {
                                                printf("Joueur inexistant\n");
                                                continue;
                                        }
                                        else if(!autrejoueur->partie) {
                                                printf("Partie inexistante\n");
                                                continue;
                                        }
                                        
                                        VERIF_CRITIQUE_ZERO(pthread_mutex_lock(&GET_PARTIE(autrejoueur)->join_mutex), "GET_JOIN: join_mutex lock ");
                                        
                                        if(autrejoueur->partie && !GET_PARTIE(autrejoueur)->joueur2) {
                                                GET_PARTIE(autrejoueur)->joueur2 = client;
                                                
                                                //VERIF_CRITIQUE_ZERO(pthread_mutex_unlock(&GET_PARTIE(autrejoueur)->join_mutex), "GET_JOIN: join_mutex unlock ");
                                                
                                                client->partie = autrejoueur->partie;
                                                printf("OK\n");
                                        }
                                        else {
                                                printf("Partie pleine\n");
                                        }
                                        VERIF_CRITIQUE_ZERO(pthread_mutex_unlock(&GET_PARTIE(autrejoueur)->join_mutex), "GET_JOIN: join_mutex unlock ");

                                } break;
                        
                        case GET_OBSERVE: {
                                        struct mtp_get_observe mgo;
                                        struct s_client *autrejoueur;
                                        struct s_partie *partie;
                                        mtp_read(client->sockfd, &mgo, sizeof(struct mtp_get_observe));
                                        
                                        printf("%s: Observe la partie de %s ... ", client->pseudo, mgo.pseudo);
                                        autrejoueur = (struct s_client *) liste_rechercher_obj(mgo.pseudo, 0, PSEUDO_LEN, &clients);


                                        if(!autrejoueur || !autrejoueur->partie) {
                                                printf("Joueur / partie inexistant\n");
                                                continue;
                                        }
                                        
                                        partie = LISTE_GET(struct s_partie, autrejoueur->partie);
                                        
                                        VERIF_CRITIQUE_ZERO(pthread_mutex_lock(&GET_PARTIE(autrejoueur)->winner_mutex), "GET_OBSERVE: winner_mutex lock ");
                                        
                                        if(partie->joueur1 && partie->joueur2) {
                                                liste_insert((struct s_client *)client, & GET_PARTIE(autrejoueur)->observateurs);
                                                
                                                client->status = STATUS_EN_OBSERVATION;
                                                
                                                
                                                send_push_move_extended(client->sockfd,
                                                                        GET_PARTIE(autrejoueur)->joueur1->coord.x,
                                                                        GET_PARTIE(autrejoueur)->joueur1->coord.y,
                                                                        GET_PARTIE(autrejoueur)->joueur1->caractere);
                                                
                                                if(GET_PARTIE(autrejoueur)->joueur2)
                                                        send_push_move_extended(client->sockfd,
                                                                        GET_PARTIE(autrejoueur)->joueur2->coord.x,
                                                                        GET_PARTIE(autrejoueur)->joueur2->coord.y,
                                                                        GET_PARTIE(autrejoueur)->joueur2->caractere);
        
                                        }
                                        
                                        VERIF_CRITIQUE_ZERO(pthread_mutex_unlock(&GET_PARTIE(autrejoueur)->winner_mutex), "GET_OBSERVE: winner_mutex lock ");
                                        
                                } break;
                        
                        case GET_CHALLENGE: {
                                        struct mtp_get_challenge gc;
                                        struct s_client *autrejoueur;
                                        mtp_read(client->sockfd, &gc, sizeof(struct mtp_get_challenge));
                                        
                                        client->caractere = gc.caractere;
                                        
                                        printf("%s: rejoint %s sur lab %d en utilisant le bonhomme %c\n",
                                                        client->pseudo, gc.pseudo, gc.lab_id, client->caractere);
                                        
                                        autrejoueur = (struct s_client *) liste_rechercher_obj(gc.pseudo, 0, PSEUDO_LEN, &clients);

                                        if(!client->partie && (!autrejoueur || !autrejoueur->partie || GET_PARTIE(autrejoueur)->lab_id != gc.lab_id)) {
                                                printf("Joueur / Partie inexistant ou modifiée, création de l'equivalent\n");
                                                create_partie(gc.lab_id, client);
                                        }
                                } break;
                        case GET_MAZE: {
                                        struct mtp_get_maze gm;
                                        mtp_read(client->sockfd, &gm, sizeof(struct mtp_get_maze));
                                        
                                        printf("%s: Demande du lab %d\n", client->pseudo, gm.lab_id);
                                        send_labyrinthe(client->sockfd, gm.lab_id);
                                } break;
                        case GET_MOVE: {
                                        struct mtp_get_move gm;
                                        struct s_client *adversaire;
                                        mtp_read(client->sockfd, &gm, sizeof(struct mtp_get_move));
                                        
                                        memcpy(&client->coord, &gm.coord, sizeof(struct s_coordonnees));
                                        if(client->partie && (adversaire = GET_ADVERSAIRE(client))) {
                                                send_push_move(adversaire->sockfd, (struct mtp_push_move *) &gm);
                                                client->nb_dep ++;
                                                liste_foreach(&GET_PARTIE(client)->observateurs, propager_deplacement_observateurs, client);
                                        }
                                        else 
                                                printf("%s: Change de position mais n'est pas dans une partie !\n", client->pseudo);
                                } break;
                        /*
                         * Quand un joueur gagne.
                         * Mutex necessaire pour empecher l'enchaînement suivant :
                         *      J1 envoie GET_WINNER, J1 enregistre partie dans BDD
                         *      J2 envoie GET_WINNER, J2 enregistre dans BDD
                         *      J1 reset sa partie et PUSH à J2
                         *      ... 
                         */
                        case GET_WINNER: {
                                        struct mtp_get_winner gw;
                                        struct s_client *adversaire;
                                        struct liste_elem *el_liste = NULL;
                                        int gagne = 0;
                                        mtp_read(client->sockfd, &gw, sizeof(struct mtp_get_winner));
                                        
                                        printf("%s: Partie gagnée de %s en %.2fs!\n", client->pseudo, gw.pseudo, ntohs(gw.duree) / 100.);
                                        
                                        if(!client->partie) {
                                                printf("%s: pas de partie ...\n", client->pseudo);
                                                break;
                                        }
                                        el_liste = client->partie;
                                        
                                        VERIF_CRITIQUE_ZERO(pthread_mutex_lock(&GET_PARTIE(client)->winner_mutex), "GET_WINNER: winner_mutex lock ");
                                        if(client->partie && (adversaire = GET_ADVERSAIRE(client))) {
                                                gagne = 1;
                                                /* Enregistrement en BDD */
                                                ajouter_partie(htons(gw.duree), client->nb_dep, client->pseudo,
                                                               adversaire->pseudo, GET_PARTIE(client)->lab_id);
                                                
                                                /* Envoi du gagnant aux perdants et observateurs */
                                                send_push_winner(adversaire->sockfd, gw.pseudo);
                                                
                                                liste_foreach(&GET_PARTIE(client)->observateurs, propager_vainqueur_observateurs, gw.pseudo);
                                                
                                                /* Reset / Libération des ressources */
                                                client->partie = NULL;
                                                adversaire->partie = NULL;
                                        }
                                        else
                                                printf("%s: Il a gagné avant !\n", client->pseudo);
                                        
                                        VERIF_CRITIQUE_ZERO(
                                                pthread_mutex_unlock(&LISTE_GET(struct s_partie, el_liste)->winner_mutex),
                                                "GET_WINNER: winner_mutex unlock ");
                                        
                                        if(gagne)
                                                free_partie(el_liste);
                                } break;
                                
                        case GET_ABANDON: {
                                        struct s_client *adversaire;
                                        struct liste_elem *el_liste = NULL;
                                        int gagne = 0;
                                        
                                        struct mtp_get_abandon ga;
                                        mtp_read(client->sockfd, &ga, sizeof(struct mtp_get_abandon));
                                        
                                        printf("%s: Abandon\n", client->pseudo);
                                        remove_client_from_game(cli_elem);
                                        
                                        if(!client->partie) {
                                                printf("%s: pas de partie ...\n", client->pseudo);
                                                break;
                                        }
                                        el_liste = client->partie;
                                        
                                        VERIF_CRITIQUE_ZERO(pthread_mutex_lock(&GET_PARTIE(client)->winner_mutex), "GET_WINNER: winner_mutex lock ");
                                        if(client->partie && (adversaire = GET_ADVERSAIRE(client))) {
                                                gagne = 1;
                                                
                                                /* Envoi du gagnant aux perdants et observateurs */
                                                send_push_abandon(adversaire->sockfd);
                                                
                                                liste_foreach(&GET_PARTIE(client)->observateurs, propager_abandon_observateurs, NULL);
                                                
                                                /* Reset / Libération des ressources */
                                                client->partie = NULL;
                                                adversaire->partie = NULL;
                                        }
                                        else
                                                printf("%s: Il a gagné avant !\n", client->pseudo);
                                        
                                        VERIF_CRITIQUE_ZERO(
                                                pthread_mutex_unlock(&LISTE_GET(struct s_partie, el_liste)->winner_mutex),
                                                "GET_WINNER: winner_mutex unlock ");
                                        
                                        if(gagne)
                                                free_partie(el_liste);
                                        
                                } break;
                }
        }
        while(code);
        printf("%s: Bye ! \n", client->pseudo);
        
        remove_client_from_game(cli_elem);
        free_client(cli_elem);
}


