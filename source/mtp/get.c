#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "mtp.h"

/******************************* REC *******************************************/

void    send_get_rec(int sockfd, char * pseudo) {
        struct mtp_get_rec mgr;
        memset( (struct mtp_get_rec *) &mgr,
                0, sizeof(struct mtp_get_rec));
        
        strncpy(mgr.pseudo, pseudo, PSEUDO_LEN);
        
        mtp_send(sockfd, GET_REC, &mgr, sizeof(struct mtp_get_rec));
}

/**************************** CONNECT *************************************/

void    send_get_connect(int sockfd, char *pseudo) {
        
        struct mtp_get_connect mgc;
        memset( (struct mtp_get_connect *) &mgc,
                0, sizeof(struct mtp_get_connect));
        strncpy(mgc.pseudo, pseudo, PSEUDO_LEN);
        
        mtp_send(sockfd, GET_CONNECT, &mgc, sizeof(struct mtp_get_connect));
}

/**************************** DISCONNECT *************************************/

void    send_get_disconnect(int sockfd, char pseudo[PSEUDO_LEN]) {
        
        struct mtp_get_disconnect mgd;
        memset( (struct mtp_get_disconnect *) &mgd,
                0, sizeof(struct mtp_get_disconnect));
        strncpy(mgd.pseudo, pseudo, PSEUDO_LEN);
        
        mtp_send(sockfd, GET_DISCONNECT, &mgd, sizeof(struct mtp_get_disconnect));
}

/**************************** INFO MAZES *************************************/

void    send_get_info_mazes(int sockfd) {

        struct mtp_get_info_mazes mgim;
        memset( (struct mtp_get_info_mazes *) &mgim,  0, sizeof(struct mtp_get_info_mazes));
        mtp_send(sockfd, GET_INFO_MAZES, &mgim, sizeof(struct mtp_get_info_mazes));
}

/**************************** INFO GAMES *************************************/

void    send_get_info_games(int sockfd) {

        struct mtp_get_info_games mgig;
        memset( (struct mtp_get_info_games *) &mgig,  0, sizeof(struct mtp_get_info_games));
        mtp_send(sockfd, GET_INFO_GAMES, &mgig, sizeof(struct mtp_get_info_games));
}

/******************************* OBSERVE **************************************/

void    send_get_observe(int sockfd, const char *pseudo) {
        struct mtp_get_observe mgo;
        
        memset( (struct mtp_get_observe *) &mgo,  0, sizeof(struct mtp_get_observe));
        
        strncpy(mgo.pseudo, pseudo, PSEUDO_LEN);
        
        mtp_send(sockfd, GET_OBSERVE, &mgo, sizeof(struct mtp_get_observe));
}

/************************** GET SELECTED MAZE ********************************/

void    send_get_selected_maze(int sockfd, int lab_id) {
        struct mtp_get_selected_maze mgsm;
        memset( (struct mtp_get_selected_maze *) &mgsm,  0, sizeof(struct mtp_get_selected_maze));
        mgsm.lab_id = lab_id;
        mtp_send(sockfd, GET_SELECTED_MAZE, &mgsm, sizeof(struct mtp_get_selected_maze));
}

/******************************** GET MAZE ************************************/

void    send_get_maze(int sockfd, int lab_id) {
        struct mtp_get_maze mgm;
        memset( (struct mtp_get_maze *) &mgm,  0, sizeof(struct mtp_get_maze));
        mgm.lab_id = lab_id;
        mtp_send(sockfd, GET_MAZE, &mgm, sizeof(struct mtp_get_maze));
}

/**************************** GET MOVE *************************************/

void    send_get_move(int sockfd, struct mtp_get_move *pdu) {
        mtp_send(sockfd, GET_MOVE, pdu, sizeof(struct mtp_get_move));
}

/**************************** GET ABANDON *************************************/
void    send_get_abandon(int sockfd) {
        struct mtp_get_abandon pdu;
        memset(&pdu, 0, sizeof(struct mtp_get_abandon));
        mtp_send(sockfd, GET_ABANDON, &pdu, sizeof(struct mtp_get_abandon));
}


/**************************** GET HIST MAZE *************************************/

void    send_get_hist_maze(int sockfd, int lab_id) {
        struct mtp_get_hist_maze pdu;

        memset( (struct mtp_get_hist_maze *) &pdu,
                0, sizeof(struct mtp_get_hist_maze));

        pdu.lab_id = (byte_t) lab_id;

        mtp_send(sockfd, GET_HIST_MAZE, &pdu, sizeof(struct mtp_get_hist_maze));
}


/**************************** GET HIST PLAYER *************************************/

void    send_get_hist_player(int sockfd, const char *pseudo) {
        struct mtp_get_hist_player pdu;

        memset( (struct mtp_get_hist_player *) &pdu,
                0, sizeof(struct mtp_get_hist_player));

        strncpy(pdu.pseudo, pseudo, PSEUDO_LEN);

        mtp_send(sockfd, GET_HIST_PLAYER, &pdu, sizeof(struct mtp_get_hist_player));
}


/**************************** GET CHALLENGE *************************************/

void    send_get_challenge(int sockfd, const char *pseudo, 
                        char caractere, int lab_id, int niveau )
{
        struct mtp_get_challenge pdu;

        memset( (struct mtp_get_challenge *) &pdu,
                0, sizeof(struct mtp_get_challenge));

        pdu.caractere = caractere ;
        pdu.lab_id = (byte_t) lab_id;
        pdu.niveau = (byte_t) niveau;
        
        if(pseudo)
                strncpy(pdu.pseudo, pseudo, PSEUDO_LEN);

        mtp_send(sockfd, GET_CHALLENGE, &pdu, sizeof(struct mtp_get_challenge));
}


/**************************** GET MESSAGE *************************************/

void    send_get_message(int sockfd, const char *pseudo, 
                                     const char *msg) 
{
        struct mtp_get_message pdu;

        memset( (struct mtp_get_message *) &pdu,
                0, sizeof(struct mtp_get_message));

        strncpy(pdu.pseudo, pseudo, PSEUDO_LEN);
        strncpy(pdu.message.message, msg, MESSAGE_LEN);
        pdu.message.len = (byte_t) (strlen(msg) % MESSAGE_LEN);

        mtp_send(sockfd, GET_MESSAGE, &pdu, sizeof(struct mtp_get_message));
}


/**************************** GET ENDED GAME *************************************/

void    send_get_ended_game(int sockfd, int partie_id) {
        struct mtp_get_ended_game pdu;

        memset( (struct mtp_get_ended_game *) &pdu,
                0, sizeof(struct mtp_get_ended_game));

        pdu.partie_id = (byte_t) partie_id;

        mtp_send(sockfd, GET_ENDED_GAME, &pdu, sizeof(struct mtp_get_ended_game));
}


/**************************** GET CHALLENGERS *************************************/

void    send_get_challengers(int sockfd) {
        struct mtp_get_challengers pdu;

        memset( (struct mtp_get_challengers *) &pdu,
                0, sizeof(struct mtp_get_challengers));

        mtp_send(sockfd, GET_CHALLENGERS, &pdu, sizeof(struct mtp_get_challengers));
}


/**************************** GET JOIN *************************************/

void    send_get_join(int sockfd, char pseudo[PSEUDO_LEN]) {
        struct mtp_get_join pdu;

        memset( (struct mtp_get_join *) &pdu,
                0, sizeof(struct mtp_get_join));

        strncpy(pdu.pseudo, pseudo, PSEUDO_LEN);
        DEBUG("APRES COPE\n");

        mtp_send(sockfd, GET_JOIN, &pdu, sizeof(struct mtp_get_join));
        DEBUG("ARPPRES SEND\n");
}


/**************************** GET WINNER *************************************/

void    send_get_winner(int sockfd, const char *pseudo, int duree) {
        struct mtp_get_winner pdu;

        memset( (struct mtp_get_winner *) &pdu,
                0, sizeof(struct mtp_get_winner));

        pdu.duree = htons(duree);
        strncpy(pdu.pseudo, pseudo, PSEUDO_LEN);

        mtp_send(sockfd, GET_WINNER, &pdu, sizeof(struct mtp_get_winner));
}



