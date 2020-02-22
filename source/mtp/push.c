#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "mtp.h"


/******************************* REC *******************************************/

void    send_push_rec(int sockfd, int status) {
        struct mtp_push_rec mpc;
        mpc.status = (byte_t)status;
        mtp_send(sockfd, PUSH_REC, &mpc, sizeof(struct mtp_push_rec));
}


/**************************** CONNECT *************************************/

void    send_push_connect(int sockfd, int status, const char *msg) {
        
        struct mtp_push_connect mpc;
        memset( (struct mtp_push_connect *) &mpc,
                0, sizeof(struct mtp_push_connect));
        
        if(msg) {
                strncpy(mpc.message.message, msg, MESSAGE_LEN);
                mpc.message.len = (byte_t) (strlen(msg) % MESSAGE_LEN);
        }
        mpc.status = (byte_t)status;
        
        mtp_send(sockfd, PUSH_CONNECT, &mpc, sizeof(struct mtp_push_connect));
}


/************************** DISCCONNECT ***********************************/

void    send_push_disconnect(int sockfd, const char *msg) {
        
        struct mtp_push_disconnect mpd;
        memset( (struct mtp_push_disconnect *) &mpd,
                0, sizeof(struct mtp_push_disconnect));
        
        if(msg) {
                strncpy(mpd.message.message, msg, MESSAGE_LEN);
                mpd.message.len = (byte_t) (strlen(msg) % MESSAGE_LEN);
        }
        
        mtp_send(sockfd, PUSH_DISCONNECT, &mpd, sizeof(struct mtp_push_disconnect));
}

/**************************** INFO MAZES *************************************/

void    send_push_info_mazes(int sockfd, int lab_id, int niveau, int more_flag) {
        struct mtp_push_info_mazes   mpim;
        
        
        memset( (struct mtp_push_info_mazes *) &mpim,
                0, sizeof(struct mtp_push_info_mazes));
        
        mpim.lab_id = (byte_t) lab_id;
        mpim.niveau = (byte_t) niveau;
        mpim.more_flag = (byte_t) more_flag;
        
        mtp_send(sockfd, PUSH_INFO_MAZES, &mpim, sizeof(struct mtp_push_info_mazes));
}

/**************************** PUSH MAZE *************************************/

void    send_push_maze(int sockfd, int lignes, int col) {
        struct mtp_push_maze pdu;

        memset( (struct mtp_push_maze *) &pdu,
                0, sizeof(struct mtp_push_maze));

        pdu.lignes   = (byte_t) lignes;
        pdu.colonnes = (byte_t) col;

        mtp_send(sockfd, PUSH_MAZE, &pdu, sizeof(struct mtp_push_maze));
}

/**************************** PUSH LINE  *************************************/

void    send_push_line(int sockfd, char *ligne, size_t len) {
        struct mtp_push_line pdu;

        memset( (struct mtp_push_line *) &pdu,
                0, sizeof(struct mtp_push_line));

        memcpy(pdu.ligne, ligne, len);

        mtp_send(sockfd, PUSH_LINE, &pdu, sizeof(struct mtp_push_line));
}


/**************************** PUSH MOVE *************************************/

void    send_push_move(int sockfd, struct mtp_push_move *pdu) {
        mtp_send(sockfd, PUSH_MOVE, pdu, sizeof(struct mtp_push_move));
}

/**************************** PUSH MOVE ETENDU *************************************/

void    send_push_move_extended(int sockfd, int x, int y, char caractere) {
        struct mtp_push_move_extended mpme;
        
        memset(&mpme, 0, sizeof(struct mtp_push_move_extended));
        
        mpme.base.coord.x = (byte_t) x;
        mpme.base.coord.y = (byte_t) y;
        mpme.caractere = caractere;
        
        mtp_send(sockfd, PUSH_MOVE_EXTENDED, &mpme, sizeof(struct mtp_push_move_extended));
}


/**************************** PUSH HIST *************************************/

void    send_push_hist(int sockfd, int partie_id, int nb_deplacements,
                                byte2_t duree, byte_t more_fl)
{
        struct mtp_push_hist pdu;

        memset( (struct mtp_push_hist *) &pdu,
                0, sizeof(struct mtp_push_hist));

        pdu.partie_id  = (byte_t) partie_id;
        pdu.nombre_dep = (byte_t) nb_deplacements;
        pdu.duree      = htons(duree);
        pdu.more_flag  = (byte_t) more_fl;

        mtp_send(sockfd, PUSH_HIST, &pdu, sizeof(struct mtp_push_hist));
}


/**************************** PUSH MESSAGE *************************************/

void    send_push_message(int sockfd, const char *pseudo, const char *msg) {
        struct mtp_push_message pdu;

        memset( (struct mtp_push_message *) &pdu,
                0, sizeof(struct mtp_push_message));

        strncpy(pdu.pseudo, pseudo, PSEUDO_LEN);
        strncpy(pdu.message.message, msg, MESSAGE_LEN);
        pdu.message.len = (byte_t) (strlen(msg) % MESSAGE_LEN);

        mtp_send(sockfd, PUSH_MESSAGE, &pdu, sizeof(struct mtp_push_message));
}


/*************************** PUSH INFO GAMES ***********************************/

void    send_push_info_games(int sockfd, const char *pseudo1, const char *pseudo2, byte_t lab_id, byte_t more_fl) {
        struct mtp_push_info_games pdu;

        memset( (struct mtp_push_info_games *) &pdu,
                0, sizeof(struct mtp_push_info_games));

        strncpy(pdu.pseudo1, pseudo1, PSEUDO_LEN);
        strncpy(pdu.pseudo2, pseudo2, PSEUDO_LEN);
        pdu.lab_id = (byte_t) lab_id;
        pdu.more_flag = (byte_t) more_fl;

        mtp_send(sockfd, PUSH_INFO_GAMES, &pdu, sizeof(struct mtp_push_info_games));
}

/**************************** PUSH ENDED GAME *************************************/

void    send_push_ended_game(int sockfd, const char *pseudo_gagnant,
                        const char *pseudo_perdant, int lab_id)
{
        struct mtp_push_ended_game pdu;

        memset( (struct mtp_push_ended_game *) &pdu,
                0, sizeof(struct mtp_push_ended_game));

        pdu.lab_id = (byte_t) lab_id;
        strncpy(pdu.pseudo_gagnant, pseudo_gagnant, PSEUDO_LEN);
        strncpy(pdu.pseudo_perdant, pseudo_perdant, PSEUDO_LEN);

        mtp_send(sockfd, PUSH_ENDED_GAME, &pdu, sizeof(struct mtp_push_ended_game));
}


/**************************** PUSH CHALLENGERS *************************************/

void    send_push_challengers(int sockfd, const char *pseudo,
                              int lab_id, int niveau, int more_fl)
{
        struct mtp_push_challengers pdu;

        memset( (struct mtp_push_challengers *) &pdu,
                0, sizeof(struct mtp_push_challengers));

        strncpy(pdu.pseudo, pseudo, PSEUDO_LEN);
        pdu.lab_id = (byte_t) lab_id;
        pdu.niveau = (byte_t) niveau;
        pdu.more_flag = (byte_t) more_fl;

        mtp_send(sockfd, PUSH_CHALLENGERS, &pdu, sizeof(struct mtp_push_challengers));
}


/**************************** PUSH WINNER *************************************/

void    send_push_winner(int sockfd, const char *pseudo) {
        struct mtp_push_winner pdu;

        memset( (struct mtp_push_winner *) &pdu,
                0, sizeof(struct mtp_push_winner));

        strncpy(pdu.pseudo, pseudo, PSEUDO_LEN);

        mtp_send(sockfd, PUSH_WINNER, &pdu, sizeof(struct mtp_push_winner));
}


/**************************** PUSH ABANDON *************************************/

void    send_push_abandon(int sockfd) {
        struct mtp_push_abandon pdu;
        memset(&pdu, 0, sizeof(struct mtp_push_abandon));
        mtp_send(sockfd, PUSH_ABANDON, &pdu, sizeof(struct mtp_push_abandon));
}


