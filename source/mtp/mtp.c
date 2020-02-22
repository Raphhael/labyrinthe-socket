#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>

#include "mtp.h"
#include "../constants.h"

extern int errno;

code_t  mtp_next_code(int sockfd, int flags) {
        code_t lu;
        int    recu;
        
        recu = recv(sockfd, &lu, sizeof(code_t), flags);
        
        if(!recu)
                return 0;
        else if(recu == -1) {
                if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS) {
                        DEBUG("Erreur errno\n");
                        return 0;
                }
                else if(errno == ECONNRESET) {
                        DEBUG("Quit violent\n");
                        return 0;
                }
                else
                        VERIF_CRITIQUE(1, 1, "mtp_silent_get_next_code: echec lecture 1 octet ");
        }
        
        
        return lu;
}


code_t  mtp_get_next_code(int sockfd) {
        return mtp_next_code(sockfd, 0);
}

code_t  mtp_see_next_code(int sockfd) {
        return mtp_next_code(sockfd, MSG_PEEK);
}


void    mtp_send(int sockfd, int code_i, void * mtp, size_t len) {
        char   *pkt = NULL;
        code_t code = (code_t) code_i;
        int    envoye;
        
        size_t pkt_size = len + sizeof(code_t);
        
        pkt = (char *) malloc(pkt_size);
        VERIF_CRITIQUE(pkt, NULL, "mtp_send: plus de mémoire ");
        
        memcpy((char *) pkt, (code_t *) &code, sizeof(code_t));
        /* On ajoute le payload après le code */
        memcpy((char *) (pkt + sizeof(code)), mtp, len);
        
        envoye = write(sockfd, pkt, pkt_size);
        
        DEBUG("Envoi de %d !\n", code);
        
        if(envoye != pkt_size) {
                DEBUG("mtp_send: envoyé=%d, attendu=%d (", envoye, pkt_size);
                for(int i = 0; i < pkt_size; i++)
                        DEBUG("%X", pkt[i]);
                DEBUG(")\n");
        }
        
        free(pkt);
}

int     mtp_read(int sockfd, void *buffer, size_t buf_len) {
        
        memset(buffer, 0, buf_len);
        
        int bytes_lus = recv(sockfd, buffer, buf_len, 0);

        VERIF_CRITIQUE(bytes_lus, -1, "read_get_connect: echec lecture 1 octet ");
        return bytes_lus;
}


void    send_error(int sockfd, int code_pdu) {
        struct mtp_error_protocol mep;
        mep.code_pdu = (byte_t) code_pdu;
        mtp_send(sockfd, PUSH_REC, &mep, sizeof(struct mtp_error_protocol));
}


