#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "structures.h"
#include "../constants.h"

void    liste_init(size_t elem_len, struct liste *li) {
        li->first = NULL;
        li->last = NULL;
        li->elem_len = elem_len;
}

struct liste_elem* liste_insert(void *obj, struct liste *li) {
        struct liste_elem *elem = NULL;
        
        elem = (struct liste_elem *) malloc(sizeof(struct liste_elem));
        VERIF_CRITIQUE(elem, NULL, "liste_insert: plus de mémoire ");
        
        if(li->last)
                li->last->next = elem;
        
        elem->liste = li;
        elem->next = NULL;
        elem->prev = li->last;
        elem->contenu = obj;
        
        li->last = elem;
        if(!li->first)
                li->first = elem;
        
        return elem;
}

void*   liste_supprimer(struct liste_elem *elem) {
        void *obj = elem->contenu;

        if(elem->prev)
                elem->prev->next = elem->next;
        else
                elem->liste->first = elem->next;
        
        if(elem->next)
                elem->next->prev = elem->prev;
        else
                elem->liste->last = elem->prev;
        
        free(elem);
        

        return obj;
}


void *  liste_rechercher_obj(void *srch, size_t start, size_t len, struct liste *li) {
        struct liste_elem *next_el = li->first;
        
        while(next_el) {
                if(!memcmp(next_el->contenu + start, srch, len))
                        return next_el->contenu;
                else
                        next_el = next_el->next;
        }
        
        return NULL;
}

void*   liste_suppr_obj(void *obj, struct liste *li) {
        struct liste_elem *next_el = li->first;
        
        while(next_el) {
                if(next_el->contenu == obj) 
                        return liste_supprimer(next_el);
                else
                        next_el = next_el->next;
        }
        return NULL;
}

int     liste_vide(struct liste *li) {
        return li->first == NULL;
}

unsigned int liste_size(struct liste *li) {
        int i = 0;
        struct liste_elem *next_el = li->first;
        
        while(next_el) {
                next_el = next_el->next;
                i++;
        }
        return i;
}

void    liste_foreach(struct liste *li, void (*run)(void *param, void *obj, int is_last), void *param) {
        struct liste_elem *next_el = li->first;
        
        while(next_el) {
                (*run)(param, next_el->contenu, next_el->next == NULL ? 1 : 0);
                next_el = next_el->next;
        }
}

void    liste_destroy(struct liste *li, int deep_destroy) {
        struct liste_elem *next = li->first;
        struct liste_elem *tmp;
        
        while(next) {
                tmp = next->next;
                if(deep_destroy) free(liste_supprimer(next));
                else liste_supprimer(next);
                next = tmp;
        };
}


