/**
 * Définition des méthodes GET du protocole MTP
 */

#ifndef MTP_GET
#define MTP_GET

#include "mtp.h"

void    send_get_connect(int sockfd, char *pseudo);
void    send_get_disconnect(int sockfd, char *pseudo);
void    send_get_rec(int sockfd, char * pseudo);
void    send_get_info_mazes(int sockfd);
void    send_get_selected_maze(int sockfd, int lab_id);
void    send_get_maze(int sockfd, int lab_id);
void    send_get_move(int sockfd, struct mtp_get_move *pdu);
void    send_get_hist_maze(int sockfd, int lab_id);
void    send_get_hist_player(int sockfd, const char *pseudo);
void    send_get_challenge(int sockfd, const char *pseudo, char caractere, int lab_id, int niveau );
void    send_get_message(int sockfd, const char *pseudo, const char *msg);
void    send_get_ended_game(int sockfd, int partie_id);
void    send_get_challengers(int sockfd);
void    send_get_join(int sockfd, char pseudo[PSEUDO_LEN]);
void    send_get_winner(int sockfd, const char *pseudo, int duree);
void    send_get_info_games(int sockfd);
void    send_get_observe(int sockfd, const char *pseudo);
void    send_get_abandon(int sockfd);

#endif

