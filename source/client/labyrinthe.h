#ifndef H_LABYRINTHE
#define H_LABYRINTHE

void inputloop(void);
int  drawmap(void);
void confcolors(void);
void startgame(void *);
int  labyrinthe(int id, char bonhomme);
void    observer_labyrinthe(int id);
void load(int id);

#endif 

