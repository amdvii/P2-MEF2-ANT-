#ifndef FICHIER_H
#define FICHIER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ID_TAILLE 50
#define MAX_LINE_SIZE 1024

typedef struct {
    char id_usine[MAX_ID_TAILLE];
    double capacite_max;
    double volume_source;
    double volume_traite;
} Usine_donnees;

typedef struct avl {
    Usine_donnees* element;
    struct avl *fg;
    struct avl *fd;
    int equilibre;
} AVL;

// Utilitaires
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
AVL* rechercherAVL(AVL* a, char* id);

void libererAVL(AVL* a);

// --- MODIFICATION ICI : On ajoute les filtres dans les arguments ---
void traiter_fichier(const char* nom_fichier, char* filtre_station, char* filtre_conso, AVL** arbre, int* h);

void ecrire_resultats(AVL* a, FILE* flux, const char* mode);

#endif