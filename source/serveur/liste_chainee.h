#ifndef H_LISTE_CHAINEE
#define H_LISTE_CHAINEE

#include "structures.h"

/**
 * Initialisation de la liste, allocation des ressources nécessaires.
 * 
 * @var elem_len   sizeof du type de l'objet de la liste
 * @var li         liste à initialiser
 */
void               liste_init(size_t elem_len, struct liste *li);

/**
 * Insertion d'un objet dans la liste
 *
 * @var obj        Objet à inserer
 * @var li         liste dans laquelle insérer l'objet
 * @return         L' élément contenant le nouvel objet
 */
struct liste_elem* liste_insert(void *obj, struct liste *li);


/**
 * Suppression d'un élément de la liste.
 * 
 * @var elem       Element à supprimer
 * @return         L'objet supprimé
 */
void*              liste_supprimer(struct liste_elem *elem);


/**
 * Suppression d'un objet dans la liste.
 *
 * @var obj        objet à supprimer dans la liste
 * @var li         liste dans laquelle supprimer l'objet
 * @return         L'objet supprimé
 */
void*              liste_suppr_obj(void *obj, struct liste *li);


/**
 * Recherche d'une valeur d'un objet dans la liste à une certaine position.
 *
 * @var srch       une valeur à rechercher
 * @var start      offset de départ de recherche dans l'objet
 * @var            longueur pour effectuer la recherche
 * @var li         liste dans laquelle rechercher la valeur
 * @return         l'objet trouvé, ou NULL si pas trouvé
 */
void*              liste_rechercher_obj(void *srch, size_t start, size_t len, struct liste *li);


/**
 * Calcule l'emplissage de la liste
 *
 * @var li         liste
 * @return         vrai si liste vide, faux sinon
 */
int                liste_vide(struct liste *li);


/**
 * Calcule de la taille de la liste
 *
 * @var li         liste dont on veut la taille
 * @return         La taille de la liste
 */
unsigned int       liste_size(struct liste *li);


/**
 * Parcourir la liste
 *
 * @var li         liste à parcourir
 * @var run        fonction à exécuter pour chaque élément de la liste.
                   cette fonction prend un param donné par l'utilisateur, 
                   l'objet parcouru courant,et un flag = 1 si c'est
                   le dernier element de la liste, 0 sinon.
 * @var            Le parametre à passer à la fonction
 */
void               liste_foreach(struct liste *li, void (*run)(void *param, void *obj, int is_last), void *param);


/**
 * Libération des ressources
 * @var li            liste à supprimer
 * @var deep_destroy  boolean : true si on doit free l'objet, false sinon
 */
void               liste_destroy(struct liste *li, int deep_destroy);

#endif

