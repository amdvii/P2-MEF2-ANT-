#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fichier.h"

int main(int argc, char *argv[]) {
    // on verifie qu'il y a assez d'arguments
    if (argc < 4) {
        printf("Il manque des arguments : fichier csv, station, conso\n");
        return 1;
    }

    // recuperation des arguments
    char* nom_fichier = argv[1];
    char* type_station = argv[2];
    char* type_conso = argv[3];
    
    // on regarde si y a un 4eme argument pour l'id (leaks)
    char* id_station = NULL;
    if (argc >= 5) {
        id_station = argv[4];
    }

    AVL* arbre = NULL;
    int h = 0;

    // si on a un id, c'est le mode leaks
    if (id_station != NULL && strcmp(id_station, "") != 0) {
        // partie pas finie
        printf("Mode Leaks pas encore implémenté\n");
    } 
    else {
        // sinon c'est le mode histo
        // on remplit l'arbre avec les donnees du fichier
        traiter_fichier(nom_fichier, type_station, type_conso, &arbre, &h);

        // creation du fichier temporaire pour le shell
        FILE* fichier_sortie = fopen("tmp/data.csv", "w");
        if (fichier_sortie == NULL) {
            printf("Erreur lors de la creation du fichier tmp\n");
            libererAVL(arbre);
            return 1;
        }
        
        // on ecrit les resultats dedans
        ecrire_resultats(arbre, fichier_sortie, "max"); 
        
        fclose(fichier_sortie);
    }

    // on libere la memoire a la fin
    libererAVL(arbre);
    
    return 0;
}