#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fichier.h"

int main(int argc, char *argv[]) {
    // 1. Verification arguments de base
    if (argc < 3) {
        printf("Usage: %s [fichier.csv] [commande] [arg_opt]\n", argv[0]);
        return 1;
    }

    char* nom_fichier = argv[1];
    char* commande = argv[2];
    // char* sous_commande = (argc >= 4) ? argv[3] : NULL;

    AVL* arbre = NULL;
    int h = 0;

    if (strcmp(commande, "histo") == 0) {
        // Traitement pour l'histogramme des usines
        traiter_fichier(nom_fichier, &arbre, &h);

        // Ecriture dans un fichier temporaire unique
        // Le Shell se chargera de trier et de faire les graphiques
        FILE* fichier_sortie = fopen("data_usine.csv", "w");
        if (fichier_sortie == NULL) {
            perror("Erreur creation fichier sortie");
            libererAVL(arbre);
            return 2;
        }
        
        // En-tete pour information (facultatif si le shell gere)
        // fprintf(fichier_sortie, "Usine;Capacite;Source;Traite\n");
        
        ecrire_resultats(arbre, fichier_sortie);
        
        fclose(fichier_sortie);
    } 
    else if (strcmp(commande, "leaks") == 0) {
        printf("Fonctionnalite leaks non implementee dans cet exemple (voir sujet).\n");
        // Ici il faudrait une logique de graphe differente (AVL parents + Liste enfants)
    }
    else {
        printf("Commande inconnue : %s\n", commande);
        return 3;
    }

    libererAVL(arbre);
    return 0;
}