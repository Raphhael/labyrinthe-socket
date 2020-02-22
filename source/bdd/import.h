#ifndef H_BDD
#define H_BDD

/********************************************************************
 ********************* IMPORT ***************************************
 *******************************************************************/
/* Prépare la BDD et charge les infos utiles :
 * - connexion, charge les niveaux dans le tableau,  */
void	bdd_init(void);
/* Fait le commit, et se déconnecte */
void	bdd_exit(void);

/****************** Opérations sur les niveaux *********************/
/* Créé un tableau de pointeur de niveaux se terminant par NULL */
struct s_niveau **	get_niveaux(void);

/* Renvoie le nombre de niveaux dans la BDD. */
int			get_niveau_count(void); 

/* Insère des niveaux dans la BDD. */
void			creer_niveaux(void);

/* Afficher les niveaux de la variable global **niveaux */
void	afficher_niveaux();

/* Vérifie que niv est bien un id de niveau valide (dans la variable globale **niveaux) */
int	niveau_verif(int niv);


/*********** Opérations pour la création d'un labyrinthe ***********/
void    creer_labyrinthe(int lab_indice);
void    creer_ligne(int ligne_indice);
void    creer_colonne(int col_indice, char car);

/* Récupère l'id de labyrinthe le plus élevé dans la BDD */
int	get_last_lab(void);

#endif
