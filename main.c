#include "fichier.h"

// aide/usage
void afficher_usage(const char *nom_prog) {
    printf("Usage:\n");
    printf("  %s <datafile> histo <max|src|real|all>\n", nom_prog);
    printf("  %s <datafile> leaks \"<ID usine>\"\n", nom_prog);
    printf("\n");
    printf("Sorties:\n");
    printf("  - Les fichiers .dat et .png sont générés dans le dossier 'output/'\n");
    printf("    (le script shell.sh crée automatiquement ce dossier).\n");
}

int main(int argc, char **argv) {
    // vérif des args
    if (argc < 3) {
        afficher_usage(argv[0]);
        return 1;
    }

    //Arguments communs 
    const char *fichier_donnees = argv[1];
    const char *commande = argv[2];

    //les commandes pr les histos
    if (strcmp(commande, "histo") == 0) {
        // histo attend une option en plus : max|src|real|all 
        if (argc != 4) {
            fprintf(stderr, "Erreur: commande histo incomplète ou arguments en trop.\n");
            afficher_usage(argv[0]);
            return 2;
        }

        const char *option = argv[3];
        ModeHisto mode;
        const char *fichier_sortie = NULL;

        // Associer l'option (texte) au mode (enum) + au fichier de sortie 
        if (strcmp(option, "max") == 0) {
            mode = HISTO_MAX;
            fichier_sortie = "output/histo_max.dat";
        } else if (strcmp(option, "src") == 0) {
            mode = HISTO_SRC;
            fichier_sortie = "output/histo_src.dat";
        } else if (strcmp(option, "real") == 0) {
            mode = HISTO_REAL;
            fichier_sortie = "output/histo_real.dat";
        } else if (strcmp(option, "all") == 0) {
            mode = HISTO_ALL;
            fichier_sortie = "output/histo_all.dat";
        } else {
            fprintf(stderr,
                    "Erreur: option histo invalide (%s). Attendu: max|src|real|all\n",
                    option);
            return 3;
        }

        return traiter_histo(fichier_donnees, mode, fichier_sortie);
    }

    // Commande pr les leaks
    if (strcmp(commande, "leaks") == 0) {
        // leaks attend un identifiant d'usine en plus
        if (argc != 4) {
            fprintf(stderr, "Erreur: commande leaks incomplète ou arguments en trop.\n");
            afficher_usage(argv[0]);
            return 4;
        }

        return traiter_leaks(fichier_donnees, argv[3], "output/leaks.dat");
    }

    // pr gérer les commandes inconnues
    fprintf(stderr, "Erreur: commande inconnue (%s).\n", commande);
    afficher_usage(argv[0]);
    return 5;
}
