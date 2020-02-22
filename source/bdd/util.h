#ifndef H_BDD_UTIL
#define H_BDD_UTIL

void	bdd_connect(void);
void	bdd_disconnect(void);
void	ecpg_verif(const char* msg, int estCritique);

extern  pthread_mutex_t bdd_mutex;

#endif
