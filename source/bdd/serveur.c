#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "util.h"
#include "serveur.h"

#include "../constants.h"
#include "../mtp/mtp.h"
#include "../mtp/push.h"


EXEC SQL BEGIN DECLARE SECTION;
	struct s_ligne {
		int	numero;
	};

	struct s_colonne {
		int	numero;
		char	caractere;
	};
	struct s_labyrinthe {
		int id;
		varchar nom[LABYRINTHE_NOM_LEN];
		int niveau;
	};
	struct s_niveau {
		int numero;
		varchar nom[NIVEAU_NOM_LEN];
	};
	
	struct s_joueur {
	        char pseudo[PSEUDO_LEN];
		varchar nom[NIVEAU_NOM_LEN];
	};
EXEC SQL END DECLARE SECTION;


int	get_lab_count(void) {
        EXEC SQL BEGIN DECLARE SECTION;
                int lab_count = 0;
        EXEC SQL END DECLARE SECTION;
        
        
	EXEC SQL DECLARE sel_count CURSOR FOR
		SELECT count(*) FROM jeuxprogunix.labyrinthe;
	ecpg_verif("get_lab_count : Erreur DECLARE selector", 1);
	
	EXEC SQL OPEN sel_count;
	ecpg_verif("get_lab_count : Erreur OPEN selector", 1);
	
	EXEC SQL FETCH FROM sel_count INTO :lab_count;
	ecpg_verif("get_lab_count : Erreur connexion", 1);
	
	EXEC SQL CLOSE sel_count;
	
	return lab_count;
}

int     get_lab_niveau(int lab) {
        EXEC SQL BEGIN DECLARE SECTION;
                int niveau = 0;
                int lab_id = lab;
        EXEC SQL END DECLARE SECTION;
        
        
	EXEC SQL DECLARE sel_lab_niv CURSOR FOR
		SELECT niveau FROM jeuxprogunix.labyrinthe WHERE id = :lab_id;
	ecpg_verif("get_lab_niveau : Erreur DECLARE selector", 1);
	
	EXEC SQL OPEN sel_lab_niv;
	ecpg_verif("get_lab_niveau : Erreur OPEN selector", 1);
	
	EXEC SQL FETCH FROM sel_lab_niv INTO :niveau;
	ecpg_verif("get_lab_niveau : Erreur FETCH", 1);
	
	EXEC SQL CLOSE sel_lab_niv;
	
	return niveau;
}

int	check_pseudo(const char *input) {
        EXEC SQL BEGIN DECLARE SECTION;
	        struct s_joueur joueur;
                int             res = 0;
        EXEC SQL END DECLARE SECTION;
        
        
        strncpy(joueur.pseudo, input, PSEUDO_LEN);
        
	EXEC SQL DECLARE sel_pseudo CURSOR FOR
		SELECT count(*) FROM jeuxprogunix.joueur WHERE pseudo = :joueur.pseudo;
	ecpg_verif("check_pseudo : Erreur DECLARE selector", 1);
	
	EXEC SQL OPEN sel_pseudo;
	ecpg_verif("check_pseudo : Erreur OPEN selector", 1);
	
	EXEC SQL FETCH FROM sel_pseudo INTO :res;
	ecpg_verif("check_pseudo : Erreur connexion", 1);
	
	EXEC SQL CLOSE sel_pseudo;
	
	return res;
}

void    ajouter_pseudo(const char *input) {
        EXEC SQL BEGIN DECLARE SECTION;
	        struct s_joueur joueur;
        EXEC SQL END DECLARE SECTION;
        
        
        strncpy(joueur.pseudo, input, PSEUDO_LEN);
        strncpy(joueur.nom.arr, input, PSEUDO_LEN);
        joueur.nom.len = strlen(input);
        
	EXEC SQL INSERT INTO jeuxprogunix.joueur(pseudo, nom) VALUES (:joueur.pseudo, :joueur.nom);
	if(sqlca.sqlcode == ECPG_DUPLICATE_KEY) {
	        EXEC SQL ROLLBACK;
	        ecpg_verif("ajouter_pseudo : Rollback fail", 1);
	        return ;
	}
	ecpg_verif("ajouter_pseudo : Erreur lors de l'insertion", 1);
	
	EXEC SQL COMMIT;
	ecpg_verif("ajouter_pseudo : Erreur lors du commit", 1);
}


void	get_lab_dimension(int lab, struct s_dimension *dim) {
        EXEC SQL BEGIN DECLARE SECTION;
                int  rows = 0,
                     cols = 0,
                     lab_id = lab;
        EXEC SQL END DECLARE SECTION;

        /* Lignes */
	EXEC SQL DECLARE sel_count_ligne CURSOR FOR
		SELECT count(*)
		FROM jeuxprogunix.ligne
		WHERE labyrinthe_id = :lab_id
	;
	ecpg_verif("get_lab_dimension : Erreur DECLARE sel_count_ligne CURSOR", 1);
	EXEC SQL OPEN sel_count_ligne;
	ecpg_verif("get_lab_dimension : Erreur OPEN CURSOR sel_count_ligne", 1);
	EXEC SQL FETCH FROM sel_count_ligne INTO :rows;
	ecpg_verif("get_lab_dimension : Erreur connexion sel_count_ligne", 1);
	EXEC SQL CLOSE sel_count_ligne;


        /* Colonnes */
	EXEC SQL DECLARE sel_count_col CURSOR FOR
		SELECT count(*)
		FROM jeuxprogunix.colonne
		WHERE labyrinthe_id = :lab_id AND ligne_numero = 1;
	ecpg_verif("get_lab_dimension : Erreur DECLARE sel_count_col CURSOR", 1);
	
	EXEC SQL OPEN sel_count_col;
	ecpg_verif("get_lab_dimension : Erreur OPEN CURSOR sel_count_col", 1);
	
	EXEC SQL FETCH FROM sel_count_col INTO :cols;
	ecpg_verif("get_lab_dimension : Erreur connexion sel_count_col", 1);
	
	EXEC SQL CLOSE sel_count_col;


	dim->colonne = cols;
	dim->ligne = rows;
}

void    get_liste_labyrinthes(int sockfd) {
        int nb_labs = get_lab_count();
        
        EXEC SQL BEGIN DECLARE SECTION;
	        struct s_niveau     niveau;
	        struct s_labyrinthe labyrinthe;
        EXEC SQL END DECLARE SECTION;
        

	EXEC SQL DECLARE c_getall CURSOR FOR
		SELECT l.nom, l.id, n.numero, n.nom
		FROM jeuxprogunix.labyrinthe l
		INNER JOIN jeuxprogunix.niveau n
			ON l.niveau = n.numero
		ORDER BY n.numero
	;
	ecpg_verif("get_liste_labyrinthes : Erreur declare cursor", 1);

	EXEC SQL OPEN c_getall;
	ecpg_verif("get_liste_labyrinthes : Erreur open cursor", 1);

	EXEC SQL FETCH FROM c_getall INTO
		:labyrinthe.nom,
		:labyrinthe.id,
		:niveau.numero,
		:niveau.nom
	;
	ecpg_verif("get_liste_labyrinthes : Erreur FETCH", 1);
	
	for (int i = 1; sqlca.sqlcode == 0;i++) {
	        send_push_info_mazes(sockfd, labyrinthe.id, niveau.numero, i < nb_labs ? 1 : 0);

		EXEC SQL FETCH FROM c_getall INTO
			:labyrinthe.nom,
			:labyrinthe.id,
			:niveau.numero,
			:niveau.nom
		;
	}
	
	EXEC SQL CLOSE c_getall;
}

void    send_labyrinthe(int sockfd, int id) {
	struct s_dimension dim;
	char *ligne_str;
	int i, j;
	
	
	
        EXEC SQL BEGIN DECLARE SECTION;
                int lab_id = id;
	        struct s_ligne       ligne;
	        struct s_colonne     colonne;
	        char *select_col = "SELECT caractere FROM jeuxprogunix.colonne WHERE labyrinthe_id = ? AND ligne_numero = ?";
        EXEC SQL END DECLARE SECTION;
        
	VERIF_CRITIQUE_ZERO(pthread_mutex_lock(&bdd_mutex), "fail to lock bdd_mutex");
	
	// Dimensions
	printf("Get dimensions\n");
	get_lab_dimension(id, &dim);
	printf("Send dimensions\n");
	send_push_maze(sockfd, dim.ligne, dim.colonne);
	
	// Init ligne
	printf("Calloc line\n");
	ligne_str = (char *) calloc(dim.colonne + 1, sizeof(char *));
	VERIF_CRITIQUE(ligne_str, NULL, "send_labyrinthe: malloc fail ");
	
	// req
	printf("Declare sel line\n");
	EXEC SQL DECLARE sel_ligne CURSOR FOR
		SELECT numero FROM jeuxprogunix.ligne WHERE labyrinthe_id = :lab_id;
	ecpg_verif("send_labyrinthe : Erreur déclaration curseur sel_ligne", 1);

	// PREPARE LIGNE
	printf("Prepare req line\n");
	EXEC SQL PREPARE req_ligne FROM :select_col;
	ecpg_verif("send_labyrinthe : Erreur prepare requete req_ligne", 1);
	
	// CURSOR LIGNE
	printf("declare selcol \n");
	EXEC SQL DECLARE sel_col CURSOR FOR req_ligne;
	ecpg_verif("send_labyrinthe : Erreur declaration curseur sel_col", 1);


	printf("Open sel_col\n");
	EXEC SQL OPEN sel_ligne;
	ecpg_verif("send_labyrinthe : Erreur ouverture sel_ligne", 1);

        // Parcour de chaque ligne
	printf("Fetch 1 sel_ligne\n");
	EXEC SQL FETCH sel_ligne INTO :ligne.numero;
	ecpg_verif("send_labyrinthe : Erreur fetch premier req_ligne", 1);

	for (i = 0; sqlca.sqlcode == 0;i++) {
		// Parcour des colonnes de chaque ligne
		EXEC SQL OPEN sel_col USING :lab_id, :ligne.numero;
		ecpg_verif("send_labyrinthe : Erreur open sel_col", 1);
		EXEC SQL FETCH sel_col INTO :colonne.caractere;
		ecpg_verif("send_labyrinthe : Erreur premier fetch sel_col", 1);

		for (j = 0; sqlca.sqlcode == 0;j++) {
			ligne_str[j] = colonne.caractere;
			EXEC SQL FETCH sel_col INTO :colonne.caractere;
		}
		ligne_str[j] = '\0';
		
		// Envoi
		printf("ligne %d\t (%d): %s\n", i, j, ligne_str);
		send_push_line(sockfd, ligne_str, j);
		printf("Fin send_push_line\n");
		
		EXEC SQL CLOSE sel_col;
		ecpg_verif("send_labyrinthe : Erreur close sel_col", 1);

		EXEC SQL FETCH sel_ligne INTO :ligne.numero;
	}

	printf("Deallocate req ligne\n");
	EXEC SQL DEALLOCATE PREPARE req_ligne;
	
	EXEC SQL CLOSE sel_ligne;
	
	VERIF_CRITIQUE_ZERO(pthread_mutex_unlock(&bdd_mutex), "fail to lock bdd_mutex");
	
	printf("Free ligne_str\n");
	free(ligne_str);
	
	

}

void    ajouter_partie(int o_duree, int o_nb_dep_vainq, char o_gagnant[PSEUDO_LEN], char o_perdant[PSEUDO_LEN], int o_lab_id) {
        EXEC SQL BEGIN DECLARE SECTION;
                int duree = o_duree;
                int nb_dep_vainq = o_nb_dep_vainq;
                char gagnant[PSEUDO_LEN];
                char perdant[PSEUDO_LEN];
                int lab_id = o_lab_id;
        EXEC SQL END DECLARE SECTION;
        
        strncpy(gagnant, o_gagnant, PSEUDO_LEN);
        strncpy(perdant, o_perdant, PSEUDO_LEN);
        
        
	EXEC SQL
	        INSERT INTO jeuxprogunix.partie(duree, nb_deplacements_vainqueur,
	                                        joueur_gagnant, joueur_perdant,
	                                        labyrinthe_id)
                VALUES (:duree, :nb_dep_vainq, :gagnant, :perdant, :lab_id);
	ecpg_verif("ajouter_partie : Erreur lors de l'insertion", 1);
	
	EXEC SQL COMMIT;
	ecpg_verif("ajouter_partie : Erreur lors du commit", 1);
}

void    get_historique_labyrinthe(int sockfd, int lab_id) {
        EXEC SQL BEGIN DECLARE SECTION;
                int stmt_lab_id = lab_id;
                int duree;
                int nb_dep;
                int id;
                int total = 0;
        EXEC SQL END DECLARE SECTION;

        /*************************************************************/
	EXEC SQL DECLARE sel_hl_count CURSOR FOR
	        SELECT COUNT(id) FROM jeuxprogunix.partie
                WHERE labyrinthe_id = :stmt_lab_id;
	;
	ecpg_verif("get_historique_labyrinthe : Erreur DECLARE count selector", 1);
	
	EXEC SQL OPEN sel_hl_count;
	ecpg_verif("get_historique_labyrinthe : Erreur OPEN count selector", 1);
	
	EXEC SQL FETCH FROM sel_hl_count INTO :total;
	ecpg_verif("get_historique_labyrinthe : Erreur count connexion", 1);
	
	EXEC SQL CLOSE sel_hl_count;
        /*************************************************************/
        
        if(!total) {
                printf("Demande d'historique mais pas d'historique ...\n");
                return ;
        }
        
	EXEC SQL DECLARE c_get_hist_lab CURSOR FOR
	        SELECT id, duree, nb_deplacements_vainqueur
	        FROM jeuxprogunix.partie
                WHERE labyrinthe_id = :stmt_lab_id;
	;
	ecpg_verif("get_historique_labyrinthe : Erreur declare cursor", 1);
	
	EXEC SQL OPEN c_get_hist_lab;
	ecpg_verif("get_historique_labyrinthe : Erreur open cursor", 1);

	EXEC SQL FETCH FROM c_get_hist_lab INTO
		:id,
		:duree,
		:nb_dep
	;
	for (int i = 1; sqlca.sqlcode == 0;i++) {
	        send_push_hist(sockfd, id, nb_dep, duree, i < total ? 1 : 0);

	        EXEC SQL FETCH FROM c_get_hist_lab INTO
		        :id,
		        :duree,
		        :nb_dep
	        ;
	}
	
	EXEC SQL CLOSE c_get_hist_lab;
}

void    get_historique_pseudo(int sockfd, char pseudo[PSEUDO_LEN]) {
        EXEC SQL BEGIN DECLARE SECTION;
                char stmt_pseudo[PSEUDO_LEN];
                int duree;
                int nb_dep;
                int id;
                int total = 0;
        EXEC SQL END DECLARE SECTION;
        
        memcpy(stmt_pseudo, pseudo, PSEUDO_LEN);

        /*************************************************************/
	EXEC SQL DECLARE sel_hp_count CURSOR FOR
	        SELECT COUNT(id) FROM jeuxprogunix.partie
                WHERE joueur_perdant = :stmt_pseudo OR joueur_gagnant = :stmt_pseudo;
	;
	ecpg_verif("get_historique_pseudo : Erreur DECLARE count selector", 1);
	
	EXEC SQL OPEN sel_hp_count;
	ecpg_verif("get_historique_pseudo : Erreur OPEN count selector", 1);
	
	EXEC SQL FETCH FROM sel_hp_count INTO :total;
	ecpg_verif("get_historique_pseudo : Erreur count connexion", 1);
	
	EXEC SQL CLOSE sel_hp_count;
        /*************************************************************/
        
        if(!total) {
                printf("Demande d'historique mais pas d'historique ...\n");
                return ;
        }
        
	EXEC SQL DECLARE c_get_hist_pseudo CURSOR FOR
	        SELECT id, duree, nb_deplacements_vainqueur
	        FROM jeuxprogunix.partie
                WHERE joueur_perdant = :stmt_pseudo OR joueur_gagnant = :stmt_pseudo;
	;
	ecpg_verif("get_historique_pseudo : Erreur declare cursor", 1);

	EXEC SQL OPEN c_get_hist_pseudo;
	ecpg_verif("get_historique_pseudo : Erreur open cursor", 1);

	EXEC SQL FETCH FROM c_get_hist_pseudo INTO
		:id,
		:duree,
		:nb_dep
	;
	for (int i = 1; sqlca.sqlcode == 0;i++) {
	        send_push_hist(sockfd, id, nb_dep, duree, i < total ? 1 : 0);

	        EXEC SQL FETCH FROM c_get_hist_pseudo INTO
		        :id,
		        :duree,
		        :nb_dep
	        ;
	}
	
	EXEC SQL CLOSE c_get_hist_pseudo;
}

