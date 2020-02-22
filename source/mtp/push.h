/**
 * Définition des méthodes PUSH du protocole MTP
 */
#ifndef MTP_PUSH
#define MTP_PUSH

#include "mtp.h"

void    send_push_connect(int sockfd, int status, char *msg);
void    send_push_disconnect(int sockfd, char *msg);
void    send_push_rec(int sockfd, int status);
void    send_push_info_mazes(int sockfd, int lab_id, int niveau, int more_flag);
void    send_push_maze(int sockfd, int lignes, int col);
void    send_push_line(int sockfd, char *ligne, size_t len);
void    send_push_move(int sockfd, struct mtp_push_move *pdu);
void    send_push_move_extended(int sockfd, int x, int y, char caractere);
void    send_push_hist(int sockfd, int partie_id, int nb_deplacements, byte2_t duree, byte_t more_fl);
void    send_push_message(int sockfd, const char *pseudo, const char *msg);
void    send_push_ended_game(int sockfd, const char *pseudo_gagnant, const char *pseudo_perdant, int lab_id);
void    send_push_challengers(int sockfd, const char *pseudo, int lab_id, int niveau, int more_fl);
void    send_push_winner(int sockfd, const char *pseudo);
void    send_push_info_games(int sockfd, const char *pseudo1, const char *pseudo2, byte_t lab_id, byte_t more_fl);
void    send_push_abandon(int sockfd);

#endif

