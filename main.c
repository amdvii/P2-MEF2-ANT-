#include "fichier.h"

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage:\n"
        "  %s <datafile> histo <max|src|real|all>\n"
        "  %s <datafile> leaks \"<ID usine>\"\n"
        "\n"
        "Sorties:\n"
        "  - Les fichiers .dat et .png sont générés dans le dossier 'output/'\n"
        "    (le script shell.sh crée automatiquement ce dossier).\n",
        prog, prog
    );
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argv[0]);
        return 1;
    }

    const char *datafile = argv[1];
    const char *cmd = argv[2];

    if (strcmp(cmd, "histo") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Erreur: commande histo incomplète ou arguments en trop.\n");
            usage(argv[0]);
            return 2;
        }

        const char *opt = argv[3];
        ModeHisto mode;
        const char *out = NULL;

        if (strcmp(opt, "max") == 0) {
            mode = HISTO_MAX;
            out = "output/histo_max.dat";
        } else if (strcmp(opt, "src") == 0) {
            mode = HISTO_SRC;
            out = "output/histo_src.dat";
        } else if (strcmp(opt, "real") == 0) {
            mode = HISTO_REAL;
            out = "output/histo_real.dat";
        } else if (strcmp(opt, "all") == 0) {
            mode = HISTO_ALL;
            out = "output/histo_all.dat";
        } else {
            fprintf(stderr, "Erreur: option histo invalide (%s). Attendu: max|src|real|all\n", opt);
            return 3;
        }

        return traiter_histo(datafile, mode, out);
    }

    if (strcmp(cmd, "leaks") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Erreur: commande leaks incomplète ou arguments en trop.\n");
            usage(argv[0]);
            return 4;
        }
        return traiter_leaks(datafile, argv[3], "output/leaks.dat");
    }

    fprintf(stderr, "Erreur: commande inconnue (%s).\n", cmd);
    usage(argv[0]);
    return 5;
}
