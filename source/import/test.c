#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ECPG_VERIF(msg, critique)						\
	if (sqlca.sqlcode != 0) {						\
		fprintf(stderr, "%s : %s\n", msg, sqlca.sqlerrm.sqlerrmc);	\
		if(critique)							\
			exit(EXIT_FAILURE);					\
	}
	
#define	DEFAULT_LVL { "Débutant", "Intermediaire", "Confirmé", "Avancé", "Expert" }

EXEC SQL BEGIN DECLARE SECTION;
	struct s_labyrinthe {
		int numero;
		int niveau;
	};
	
	struct s_labyrinthe lab;
	int		count;
EXEC SQL END DECLARE SECTION;

int     last_lab_id = 10;

/*
void	create_niveaux() {
	char	noms[5][40]	= DEFAULT_LVL;
	
	printf("create_niveaux : Ajout des niveaux\n");
	
	for(i = 1; i < 5; i++) {
		strcpy(niv.nom.arr, noms[i]);
		niv.nom.len = strlen(noms[i]);
		niv.numero = i;
		
		EXEC SQL INSERT INTO jeuxprogunix.niveau(numero, nom) VALUES (:niv.numero, :niv.nom);
		ECPG_VERIF("create_niveaux : Erreur lors de l'insertion", 0);
	}
	
	EXEC SQL COMMIT;
	ECPG_VERIF("create_niveaux : Erreur lors du commit", 0);
}


int	getNiveauCount(void) {
	EXEC SQL DECLARE sel_count CURSOR FOR
		SELECT count(*) FROM jeuxprogunix.niveau;
	EXEC SQL OPEN sel_count;
	EXEC SQL FETCH FROM sel_count INTO :count;
	ECPG_VERIF("getNiveauCount : Erreur connexion", 1);
	EXEC SQL CLOSE sel_count;
	return count;
}

struct s_niveau **getNiveaux(void) {
	struct s_niveau **nivx;
	struct s_niveau *nivTmp;
	int		nb_niv,
			i;
	
	if(!getNiveauCount())
		create_niveaux();
	
	nb_niv = getNiveauCount();
	if(!nb_niv)
		fprintf(stderr, "getNiveaux : nb_niv toujours vide\n");
	
	nivx = (struct s_niveau **) malloc(sizeof(struct s_niveau *) * (nb_niv + 1));
	
	EXEC SQL DECLARE sel CURSOR FOR 
		SELECT numero, nom FROM jeuxprogunix.niveau;
	EXEC SQL OPEN sel;
	
	EXEC SQL FETCH FROM sel INTO :niv.numero, :niv.nom;
	ECPG_VERIF("getNiveaux : Erreur FETCH", 1);
	for (i = 0;sqlca.sqlcode == 0;i++) {
		nivTmp = (struct s_niveau *) malloc(sizeof(struct s_niveau));
		memcpy(
			(struct s_niveau *) nivTmp,
			(struct s_niveau *) &niv,
			sizeof(struct s_niveau)
		);
		nivx[i] = nivTmp;
		printf("%d %s\n", niv.numero, niv.nom.arr);
		EXEC SQL FETCH FROM sel INTO :niv.numero, :niv.nom;
	}
	nivx[i] = NULL;
	EXEC SQL CLOSE sel;
	EXEC SQL DISCONNECT;
	return nivx;
}*/

int	get_last_lab(void) {
	EXEC SQL DECLARE last CURSOR FOR
		SELECT id FROM jeuxprogunix.labyrinthe
		ORDER BY id DESC
		LIMIT 1;
	ECPG_VERIF("get_last_lab : Erreur declare cursor", 1);
	EXEC SQL OPEN last;
	ECPG_VERIF("get_last_lab : Erreur open cursor", 1);
	EXEC SQL FETCH FROM last INTO :lab.numero;
	
	if(sqlca.sqlcode == ECPG_NOT_FOUND)
		return 0;
	ECPG_VERIF("get_last_lab : Erreur FETCH", 1);
	return lab.numero;
}

int	main(void) {
	EXEC SQL CONNECT TO BDii05816@opale USER "ii05816" USING "Rapha3ll3";
	ECPG_VERIF("Erreur connexion", 1);
	
	printf("Last lab : %d\n", get_last_lab());
	
	EXEC SQL DISCONNECT;
	return 0;
}
