#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fichier.h"

int main() {
    AVL* monArbre = NULL;
    int h = 0;

    printf("Creation des donnees...\n");

    // Allocation Usine 1
    Usine_donnees* u1 = (Usine_donnees*)malloc(sizeof(Usine_donnees));
    if (u1 == NULL) {
        printf("Erreur malloc u1\n");
        exit(1);
    }
    strcpy(u1->id_usine, "Usine C");
    u1->capacite_max = 500.0;

    // Allocation Usine 2
    Usine_donnees* u2 = (Usine_donnees*)malloc(sizeof(Usine_donnees));
    if (u2 == NULL) {
        printf("Erreur malloc u2\n");
        free(u1); // On nettoie ce qu'on a déjà alloué avant de quitter
        exit(1);
    }
    strcpy(u2->id_usine, "Usine A");
    u2->capacite_max = 100.0;

    // Allocation Usine 3
    Usine_donnees* u3 = (Usine_donnees*)malloc(sizeof(Usine_donnees));
    if (u3 == NULL) {
        printf("Erreur malloc u3\n");
        free(u1);
        free(u2);
        exit(1);
    }
    strcpy(u3->id_usine, "Usine B");
    u3->capacite_max = 300.0;

    printf("Insertion dans l'AVL...\n");
    monArbre = insertionAVL(monArbre, u1, &h);
    monArbre = insertionAVL(monArbre, u2, &h);
    monArbre = insertionAVL(monArbre, u3, &h);

    printf("\n--- Affichage trie ---\n");
    affichageInfixe(monArbre);
    printf("----------------------\n");

    libererAVL(monArbre);
    
    return 0;
}