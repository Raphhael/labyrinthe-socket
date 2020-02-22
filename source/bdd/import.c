#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "import.h"
#include "../constants.h"

/********************************************************************
 ********************* IMPORT ***************************************
 *******************************************************************/
 
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
		int niveau_numero;
	};
	struct s_niveau {
		int numero;
		varchar nom[NIVEAU_NOM_LEN];
	};

	struct s_niveau		niv;
	struct s_labyrinthe	lab;
	struct s_ligne		lig;
	struct s_colonne	col;
	int			count;
EXEC SQL END DECLARE SECTION;

struct s_niveau	**niveaux;
int    		last_lab_id = 0;


void	bdd_init(void) {
	bdd_connect();
	last_lab_id = get_last_lab();
	niveaux = get_niveaux();
}

void	bdd_exit(void) {
	EXEC SQL COMMIT;
	struct s_niveau **n = niveaux;
	for(;n[0]; n++)
		free(n[0]);
	free(niveaux);
	bdd_disconnect();
}

void	creer_niveaux(void) {
	int	i;
	char	noms[NB_DEFAULT_LVL][LEN_DEFAULT_LVL] = DEFAULT_LVL;
	
	printf("creer_niveaux : Ajout des niveaux\n");
	
	for(i = 0; i < NB_DEFAULT_LVL; i++) {
		strcpy(niv.nom.arr, noms[i]);
		niv.nom.len = strlen(noms[i]);
		niv.numero = i + 1;
		
		EXEC SQL INSERT INTO jeuxprogunix.niveau(numero, nom) VALUES (:niv.numero, :niv.nom);
		ecpg_verif("creer_niveaux : Erreur lors de l'insertion", 0);
	}
	
	EXEC SQL COMMIT;
	ecpg_verif("creer_niveaux : Erreur lors du commit", 0);
}

int	get_niveau_count(void) {
	EXEC SQL DECLARE sel_count CURSOR FOR
		SELECT count(*) FROM jeuxprogunix.niveau;
	EXEC SQL OPEN sel_count;
	EXEC SQL FETCH FROM sel_count INTO :count;
	ecpg_verif("get_niveau_count : Erreur connexion", 1);
	EXEC SQL CLOSE sel_count;
	return count;
}


int	get_last_lab(void) {
	EXEC SQL DECLARE last CURSOR FOR
		SELECT id FROM jeuxprogunix.labyrinthe
		ORDER BY id DESC
		LIMIT 1;
	ecpg_verif("get_last_lab : Erreur declare cursor", 1);
	EXEC SQL OPEN last;
	ecpg_verif("get_last_lab : Erreur open cursor", 1);
	EXEC SQL FETCH FROM last INTO :lab.id;
	
	if(sqlca.sqlcode == ECPG_NOT_FOUND)
		return 0;
	ecpg_verif("get_last_lab : Erreur FETCH", 1);
	return lab.id;
}

struct s_niveau **get_niveaux(void)  {
	struct s_niveau **nivx;
	struct s_niveau *niv_tmp;
	int		nb_niv,
			i;
	
	if(!get_niveau_count())
		creer_niveaux();
	
	nb_niv = get_niveau_count();
	if(!nb_niv)
		fprintf(stderr, "getNiveaux : nb_niv toujours vide\n");
	
	nivx = (struct s_niveau **) malloc(sizeof(struct s_niveau *) * (nb_niv + 1));
	
	EXEC SQL DECLARE sel CURSOR FOR 
		SELECT numero, nom 
		FROM jeuxprogunix.niveau;
	EXEC SQL OPEN sel;
	
	EXEC SQL FETCH FROM sel INTO :niv.numero, :niv.nom;
	ecpg_verif("getNiveaux : Erreur FETCH", 1);
	for (i = 0;sqlca.sqlcode == 0;i++) {
		niv_tmp = (struct s_niveau *) malloc(sizeof(struct s_niveau));
		memcpy(
			(struct s_niveau *) niv_tmp,
			(struct s_niveau *) &niv,
			sizeof(struct s_niveau)
		);
		nivx[i] = niv_tmp;
		EXEC SQL FETCH FROM sel INTO :niv.numero, :niv.nom;
	}
	nivx[i] = NULL;
	EXEC SQL CLOSE sel;
	return nivx;
}

void	afficher_niveaux() {
	struct s_niveau **n = niveaux;
	printf("Les niveaux disponibles sont : \n");
	
	for(;n[0]; n++)
		printf("  - %d) %s\n", n[0]->numero, n[0]->nom.arr);
}

int	niveau_verif(int niv) {
	struct s_niveau **n = niveaux;
	for(;n[0]; n++){
		if(n[0]->numero == niv)
			return 1;
	}
	return 0;
}


void    creer_labyrinthe(int lab_indice) {
	char tmp;
        lab.niveau_numero = -1;
        lab.id = lab_indice + last_lab_id;
        
        /*******************************
         * Saisie du nom du labyrinthe
         ******************************/
        printf("Saisir le nom du labyrinthe : \n");
        fgets(lab.nom.arr, LABYRINTHE_NOM_LEN-1, stdin);
        lab.nom.len = strlen(lab.nom.arr);
        if(lab.nom.arr[lab.nom.len - 1] == '\n') {
		lab.nom.arr[--lab.nom.len] = '\0';
	}
	
	if(!lab.nom.len) {
		strcpy(lab.nom.arr, "No name");
		lab.nom.len = strlen(lab.nom.arr);
	}
	
	
        /*******************************
         * Saisie du niveau
         ******************************/
        afficher_niveaux();
        
        while(!niveau_verif(lab.niveau_numero)) {
                printf("Entrer le niveau du labyrinthe \"%s\": ", lab.nom.arr);
                scanf("%d", &(lab.niveau_numero));
                scanf("%c", &tmp);
        }
        
        
	EXEC SQL 
		INSERT INTO jeuxprogunix.labyrinthe(id, nom, niveau) 
		VALUES (:lab.id, :lab.nom, :lab.niveau_numero);
	ecpg_verif("creer_labyrinthe : Erreur lors de l'insertion", 1);
}

void    creer_ligne(int ligne_indice) {
	lig.numero = ligne_indice;
        
	EXEC SQL 
		INSERT INTO jeuxprogunix.ligne(numero, labyrinthe_id) 
		VALUES (:lig.numero, :lab.id);
	ecpg_verif("creer_ligne : Erreur lors de l'insertion", 1);
		
        
}


void     creer_colonne(int col_indice, char car) {
        if(car != BLANC && car != ENTREE && car != MUR_X && car != MUR_V && car != MUR_H){
                 fprintf(stderr, "Caractère \"%c\" (code %d) inconnu\n.", car, car);
                 exit(EXIT_FAILURE);
        }
        
        col.numero = col_indice;
        col.caractere = car;
	
	EXEC SQL 
		INSERT INTO jeuxprogunix.colonne(numero, caractere, ligne_numero, labyrinthe_id) 
		VALUES (:col.numero, :col.caractere, :lig.numero, :lab.id);
	ecpg_verif("creer_colonne : Erreur lors de l'insertion", 1);
}


