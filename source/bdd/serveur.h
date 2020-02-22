#ifndef H_BDD_CLIENT
#define H_BDD_CLIENT

#include "../mtp/mtp.h"

struct s_dimension {
	int ligne;
	int colonne;
};

int	check_pseudo(const char *input);
void    ajouter_pseudo(const char *input);
void    ajouter_partie(int o_duree, int o_nb_dep_vainq, char o_gagnant[PSEUDO_LEN],
                                        char o_perdant[PSEUDO_LEN], int o_lab_id);

/* Retourne le nombre de labyrinthe dispo dans la bdd */
int     get_lab_count(void);

int     get_lab_niveau(int lab);

/* Charge le format du labyrinthe dans le parametre dim */
void	get_lab_dimension(int lab, struct s_dimension *dim);

/* Retourne un tableau de chaine de caractere : liste des labyrinthes avec leur ID */
char	**get_liste_labyrinthe();
/* Besoin d'être free */
void    get_liste_labyrinthes(int sockfd);

/*
 *	Chargement d'un labyrinthe
 *	Format tableau :
 *	- Chaque entrée du tableau est une ligne du labyrinthe
 *	- Chaque ligne se termine par un \0
 *	- La derniere entrée du tableau est NULL
 */
char	**get_labyrinthe(int id);
void    send_labyrinthe(int sockfd, int id);

/* "Free"eur des fonctions  get_liste_labyrinthe()  et  get_labyrinthe() */
void	free_2(char ***tab);


void    get_historique_pseudo(int sockfd, char pseudo[PSEUDO_LEN]);
void    get_historique_labyrinthe(int sockfd, int lab_id);

#endif
