#ifndef MAZE_TRANSFERT_PROTOCOL
#define MAZE_TRANSFERT_PROTOCOL
#include <stdint.h>
/* CODES COMMANDES */
#define GET_CONNECT             05
#define PUSH_CONNECT            10
#define GET_DISCONNECT          15
#define PUSH_DISCONNECT         20
#define GET_INFO_MAZES          25
#define PUSH_INFO_MAZES         30
#define GET_SELECTED_MAZE       35
#define GET_MAZE                40
#define PUSH_MAZE               45
#define PUSH_LINE               50
#define GET_REC                 55
#define PUSH_REC                60
#define GET_MOVE                65
#define PUSH_MOVE               70
#define GET_HIST_MAZE           75
#define GET_HIST_PLAYER         80
#define PUSH_HIST               85
#define GET_CHALLENGE           90
#define GET_MESSAGE             95
#define PUSH_MESSAGE            100
#define GET_ENDED_GAME          105
#define PUSH_ENDED_GAME         110
#define GET_CHALLENGERS         115
#define PUSH_CHALLENGERS        120
#define GET_JOIN                125
#define GET_WINNER              130
#define PUSH_WINNER             135
#define ERROR_PROTOCOL          140
#define GET_INFO_GAMES          145 /* Demande des parties en cours */
#define PUSH_INFO_GAMES         150 /* Infos sur parties en cours */
#define GET_OBSERVE             155 /* Demande d'observation */
#define PUSH_MOVE_EXTENDED      160 /* Push move en incluant le caractère */
#define GET_ABANDON             165 /* Abandonner */
#define PUSH_ABANDON             170 /* Abandonner */

/* AUTRES CONSTANTES */
/* Nombre de bytes pour stocker la longueur du message */
#include "../constants.h"
#define MESSAGE_LEN_B   1
#define MESSAGE_LEN     MESSAGE_LEN_B << 8  /* Taille max d'un message */

typedef uint8_t  byte_t;
typedef uint16_t byte2_t;

#define CONNU   (byte_t)1
#define INCONNU (byte_t)0
#define OUI     (byte_t)1
#define NON     (byte_t)0

/* Format d'un message à envoyer.
 * -> On précise le padding pour pas avoir de décallage bizarre. */
typedef struct message_t {  
        byte_t  len : MESSAGE_LEN_B;
        char    message[MESSAGE_LEN];
} message_t;

/* Coordonnées du labyrinthe transmises */
struct s_coordonnees {
        byte_t  x, y;
};

typedef byte_t code_t; /* Définition du type pour le code des commandes */

/**************************************************************************/
/************************  STRUCTURE DES PAYLOADs  ************************/
/**************************************************************************/
struct mtp_get_connect {                /* GET CONNECT */
        char  pseudo[PSEUDO_LEN];
};

struct mtp_push_connect {               /* PUSH CONNECT */
        byte_t     status;
        message_t  message;
};

struct mtp_get_disconnect {             /* GET DISCONNECT */
        char  pseudo[PSEUDO_LEN];
};

struct mtp_push_disconnect {            /* PUSH DISCONNECT */
        message_t  message;
};

struct mtp_get_rec {                    /* GET REC */
        char  pseudo[PSEUDO_LEN];
};

struct mtp_push_rec {                   /* PUSH REC */
        byte_t  status;
};


struct mtp_get_info_mazes {             /* GET INFO MAZES */
        byte_t  empty;
};

struct mtp_push_info_mazes {            /* PUSH INFO MAZES */
        byte_t  lab_id;
        byte_t  niveau;
        byte_t  more_flag;
};

struct mtp_get_selected_maze {          /* GET SELECTED MAZE */
        byte_t  lab_id;
};

struct mtp_get_maze {                   /* GET MAZE */
        byte_t  lab_id;
};

struct mtp_push_maze {                  /* PUSH MAZE */
        byte_t  lignes;
        byte_t  colonnes;
};

struct mtp_push_line {                  /* PUSH LINE */
        char ligne[COLONNE_MAX];
};

struct mtp_get_move {                   /* GET MOVE */
        struct s_coordonnees coord;
};

struct mtp_push_move {                  /* PUSH MOVE */
        struct s_coordonnees coord;
};

struct mtp_push_move_extended {         /* PUSH MOVE EXTENDED */
        struct mtp_push_move base;
        char                 caractere;
};

struct mtp_get_hist_maze {              /* GET HIST MAZE */
        byte_t  lab_id;
};

struct mtp_get_hist_player {            /* GET HIST PLAYER */
        char pseudo[PSEUDO_LEN];
};

struct mtp_push_hist {                  /* PUSH HIST */
        byte_t  partie_id;
        byte_t  nombre_dep;
        byte2_t duree;
        byte_t  more_flag;
};

struct mtp_get_challenge {              /* GET CHALLENGE */
        char   pseudo[PSEUDO_LEN];
        char   caractere;
        byte_t lab_id;
        byte_t niveau;
};

struct mtp_get_message {                /* GET MESSAGE */
        char      pseudo[PSEUDO_LEN];
        message_t message;
};

struct mtp_push_message {               /* PUSH MESSAGE */
        char      pseudo[PSEUDO_LEN];
        message_t message;
};

struct mtp_get_ended_game {             /* GET ENDED GAME */
        byte_t partie_id;
};

struct mtp_push_ended_game {            /* PUSH ENDED GAME */
        char   pseudo_gagnant[PSEUDO_LEN];
        char   pseudo_perdant[PSEUDO_LEN];
        byte_t lab_id;
};

struct mtp_get_challengers {            /* GET CHALLENGERS */
        byte_t empty;
};

struct mtp_get_abandon {                /* GET ABANDON */
        byte_t empty;
};
struct mtp_push_abandon {               /* PUSH ABANDON */
        byte_t empty;
};

struct mtp_push_challengers {           /* PUSH CHALLENGERS */
        char   pseudo[PSEUDO_LEN];
        byte_t lab_id;
        byte_t niveau;
        byte_t more_flag;
};

struct mtp_get_join {                   /* GET JOIN */
        char pseudo[PSEUDO_LEN];
};

struct mtp_get_winner {                 /* GET WINNER */
        char    pseudo[PSEUDO_LEN];
        byte2_t duree;
};

struct mtp_push_winner {                /* PUSH WINNER */
        char pseudo[PSEUDO_LEN];
};

struct mtp_error_protocol {             /* ERROR PROTOCOL */
        byte_t code_pdu;
};

struct mtp_get_info_games {             /* GET INFO GAMES */
        byte_t empty;
};

struct mtp_push_info_games {            /* PUSH INFO GAMES */
        char   pseudo1[PSEUDO_LEN];
        char   pseudo2[PSEUDO_LEN];
        byte_t lab_id;
        byte_t more_flag;
};

struct mtp_get_observe {                /* GET OBSERVE */
        char   pseudo[PSEUDO_LEN];
};

/** 
 * Attend le prochain PDU et lit uniquement son code
 * Cette fonction est donc bloquante
 */
code_t  mtp_get_next_code(int sockfd);
/**
 * Même chose mais n'enlève pas le code du buffer
 */
code_t  mtp_see_next_code(int sockfd);

/**
 * Envoie un PDU
 * @var code_i  Code du PDU
 * @var mtp     Données
 * @var len     Longueur des données
 */
void    mtp_send(int sockfd, int code_i, void *mtp, size_t len);

/**
 * Lit un PDU de taille buf_len et le met dans buffer
 */
int     mtp_read(int sockfd, void *buffer, size_t buf_len);


/**
 * Envoie une réponse suite à une erreur.
 */
void    send_error(int sockfd, int code_pdu);

#endif

