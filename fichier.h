#include <stdio.h> 
#include <stdlib.h>
#define MAX_ID_TAILLE 35

typedef struct{
    char id_usine[MAX_ID_TAILLE];
    double capacite_max;
    double volume_total; 
    double volume_traiter; 
}Usine_donnees;

typedef struct avl{
    Usine_donnees usine;
    struct avl *d;
    struct avl *g;
    int equilibre;
}AVL;

void ;