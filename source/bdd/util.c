#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "../constants.h"
#include "util.h"

pthread_mutex_t bdd_mutex;

void	bdd_connect(void) {
	EXEC SQL CONNECT TO BDii05816@opale USER "ii05816" USING "Rapha3ll3";
	ecpg_verif("bdd_connect: Erreur lors de la connexion", 1);
	VERIF_CRITIQUE_ZERO(pthread_mutex_init(&bdd_mutex, NULL), "fail to init bdd_mutex");
}

void	bdd_disconnect(void) {
	VERIF_CRITIQUE_ZERO(pthread_mutex_destroy(&bdd_mutex), "fail to destroy bdd_mutex");
	EXEC SQL DISCONNECT;
	ecpg_verif("bdd_disconnect: problème lors de la déconnexion", 1);
}	
void	ecpg_verif(const char* msg, int estCritique) {
	if (sqlca.sqlcode != 0) {
		fprintf(stderr, "%s : %s\n", msg, sqlca.sqlerrm.sqlerrmc);
		if(estCritique)
			exit(EXIT_FAILURE);
	}
}

