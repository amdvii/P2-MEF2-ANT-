#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ID_TAILLE 50

typedef struct {
    char id_usine[MAX_ID_TAILLE];
    double capacite_max;
    double volume_total;
    double volume_traiter;
} Usine_donnees;

typedef struct avl {
    Usine_donnees* element;
    struct avl *fg;
    struct avl *fd;
    int equilibre;
} AVL;

int max(int a, int b);
int min(int a, int b);

// AVL
AVL* creerArbre(Usine_donnees* data);

AVL* rotationDroite(AVL* a);
AVL* rotationGauche(AVL* a);
AVL* doubleRotationGauche(AVL* a);
AVL* doubleRotationDroite(AVL* a);

AVL* equilibrerAVL(AVL* a);
AVL* insertionAVL(AVL* a, Usine_donnees* e, int* h);

void libererAVL(AVL* a);
void affichageInfixe(AVL* a);
