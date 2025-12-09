#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fichier.h"

int main(int argc, char *argv[]) {
    // Vérification des arguments
    // argv[1] : chemin fichier CSV
    // argv[2] : commande (histo ou leaks)
    // argv[3] : option (max, src, real) ou ID station
    
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <fichier.csv> <commande> <option>\n", argv[0]);
        return 1;
    }

    char* fichier_csv = argv[1];
    char* commande = argv[2];
    char* option = argv[3];

    AVL* monArbre = NULL;
    int h = 0;

    if (strcmp(commande, "histo") == 0) {
        // 1. Chargement et traitement des données
        traiter_fichier(fichier_csv, &monArbre, &h);

        // 2. Écriture du fichier de sortie pour Gnuplot
        // Le nom du fichier est imposé ou choisi, ici on écrit sur la sortie standard ou un fichier temporaire
        // Pour simplifier l'intégration shell, on va écrire dans un fichier "temp_data.csv"
        
        FILE* out = fopen("temp_data.csv", "w");
        if (out == NULL) {
            perror("Erreur création fichier temporaire");
            libererAVL(monArbre);
            return 3;
        }
        
        // En-tête (optionnel selon Gnuplot)
        // fprintf(out, "Station;Volume\n"); 
        
        ecrire_resultats(monArbre, out, option);
        
        fclose(out);
        
    } else if (strcmp(commande, "leaks") == 0) {
        printf("Fonctionnalité leaks à implémenter...\n");
        // Logique pour leaks ici
    } else {
        fprintf(stderr, "Commande inconnue : %s\n", commande);
        return 1;
    }

    libererAVL(monArbre);
    return 0;
}