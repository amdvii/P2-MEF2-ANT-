#include "fichier.h"


int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int min(int a, int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

AVL* creerArbre(Usine_donnees* data) {
    AVL* noeud = malloc(sizeof(AVL));
    if (noeud == NULL) {
        exit(1);
    }
    noeud->element = data;
    noeud->fg = NULL;
    noeud->fd = NULL;
    noeud->equilibre = 0;
    return noeud;
}

AVL* rotationGauche(AVL* a) {
    AVL* pivot = a->fd;
    int eq_a = a->equilibre;
    int eq_p = pivot->equilibre;

    a->fd = pivot->fg;
    pivot->fg = a;

    a->equilibre = eq_a - 1 - max(eq_p, 0);
    pivot->equilibre = eq_p - 1 + min(a->equilibre, 0);

    return pivot;
}

AVL* rotationDroite(AVL* a) {
    AVL* pivot = a->fg;
    int eq_a = a->equilibre;
    int eq_p = pivot->equilibre;

    a->fg = pivot->fd;
    pivot->fd = a;

    a->equilibre = eq_a + 1 - min(eq_p, 0);
    pivot->equilibre = eq_p + 1 + max(a->equilibre, 0);

    return pivot;
}

AVL* doubleRotationGauche(AVL* a) {
    a->fd = rotationDroite(a->fd);
    return rotationGauche(a);
}

AVL* doubleRotationDroite(AVL* a) {
    a->fg = rotationGauche(a->fg);
    return rotationDroite(a);
}

AVL* equilibrerAVL(AVL* a) {
    if (a->equilibre >= 2) {
        if (a->fd->equilibre >= 0) {
            return rotationGauche(a);
        } else {
            return doubleRotationGauche(a);
        }
    }
    else if (a->equilibre <= -2) {
        if (a->fg->equilibre <= 0) {
            return rotationDroite(a);
        } else {
            return doubleRotationDroite(a);
        }
    }
    return a;
}

AVL* insertionAVL(AVL* a, Usine_donnees* e, int* h) {
    if (a == NULL) {
        *h = 1;
        return creerArbre(e);
    }

    int cmp = strcmp(e->id_usine, a->element->id_usine);

    if (cmp < 0) {
        a->fg = insertionAVL(a->fg, e, h);
        *h = -(*h);
    }
    else if (cmp > 0) {
        a->fd = insertionAVL(a->fd, e, h);
    }
    else {
        *h = 0;
        return a;
    }

    if (*h != 0) {
        a->equilibre = a->equilibre + *h;
        
        if (a->equilibre == 0) {
            *h = 0;
        } else {
            *h = 1;
        }
    }

    if (a->equilibre >= 2 || a->equilibre <= -2) {
        a = equilibrerAVL(a);
        *h = 0;
    }

    return a;
}

void libererAVL(AVL* a) {
    if (a != NULL) {
        libererAVL(a->fg);
        libererAVL(a->fd);
        free(a->element);
        free(a);
    }
}

void affichageInfixe(AVL* a) {
    if (a != NULL) {
        affichageInfixe(a->fg);
        printf("%s (Eq: %d) - Cap: %.2f\n", a->element->id_usine, a->equilibre, a->element->capacite_max);
        affichageInfixe(a->fd);
    }
}

#include "fichier.h"

// ... (Gardez vos fonctions max, min, AVL, etc. au début du fichier) ...

// Fonction de recherche (à ajouter avant traiter_fichier)
AVL* rechercherAVL(AVL* a, char* id) {
    if (a == NULL) {
        return NULL;
    }
    int cmp = strcmp(id, a->element->id_usine);
    if (cmp == 0) {
        return a;
    } else if (cmp < 0) {
        return rechercherAVL(a->fg, id);
    } else {
        return rechercherAVL(a->fd, id);
    }
}

void traiter_fichier(const char* nom_fichier, AVL** arbre, int* h) {
    FILE* file = fopen(nom_fichier, "r");
    if (file == NULL) {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s\n", nom_fichier);
        exit(2);
    }

    char ligne[MAX_LINE_SIZE];

    while (fgets(ligne, sizeof(ligne), file)) {
        // strtok modifie la chaîne, on récupère les colonnes une par une
        char* col1 = strtok(ligne, ";");
        (void)col1; // CORRECTION : On ignore col1 pour l'instant pour éviter le warning

        char* col2 = strtok(NULL, ";");
        char* col3 = strtok(NULL, ";");
        char* col4 = strtok(NULL, ";");
        char* col5 = strtok(NULL, ";");

        // On vérifie qu'on a bien les colonnes minimales pour travailler
        if (!col2 || !col3 || !col4) continue;

        // Cas 1 : Définition d'une usine (Capacité)
        if (strstr(col2, "Plant") != NULL || strstr(col2, "Facility") != NULL) {
             if (strcmp(col3, "-") == 0) {
                 AVL* noeud = rechercherAVL(*arbre, col2);
                 if (noeud == NULL) {
                     Usine_donnees* u = malloc(sizeof(Usine_donnees));
                     strcpy(u->id_usine, col2);
                     u->capacite_max = atof(col4);
                     u->volume_source = 0;
                     u->volume_traite = 0;
                     *arbre = insertionAVL(*arbre, u, h);
                 } else {
                     noeud->element->capacite_max = atof(col4);
                 }
             }
        }

        // Cas 2 : Apport Source -> Usine
        if ((strstr(col3, "Plant") != NULL || strstr(col3, "Facility") != NULL) && strcmp(col2, "-") != 0) {
            char* id_usine = col3;
            double volume = atof(col4);
            
            // CORRECTION : On déclare rendement, mais on le caste en (void) pour éviter le warning
            double rendement = (col5 && strcmp(col5, "-") != 0) ? atof(col5) : 0.0;
            (void)rendement; // On s'en servira plus tard pour l'option "real"

            AVL* noeud = rechercherAVL(*arbre, id_usine);
            if (noeud == NULL) {
                Usine_donnees* u = malloc(sizeof(Usine_donnees));
                strcpy(u->id_usine, id_usine);
                u->capacite_max = 0;
                u->volume_source = volume;
                u->volume_traite = 0;
                *arbre = insertionAVL(*arbre, u, h);
            } else {
                noeud->element->volume_source += volume;
            }
        }
    }
    fclose(file);
}

// Parcours infixe pour écrire dans le fichier de sortie
void ecrire_resultats(AVL* a, FILE* flux, const char* mode) {
    if (a != NULL) {
        ecrire_resultats(a->fg, flux, mode);
        
        double valeur = 0.0;
        if (strcmp(mode, "max") == 0) valeur = a->element->capacite_max;
        else if (strcmp(mode, "src") == 0) valeur = a->element->volume_source;
        // else if (strcmp(mode, "real") == 0) valeur = a->element->volume_traite;

        // On n'écrit que si la valeur est pertinente (non nulle)
        if (valeur > 0) {
            fprintf(flux, "%s;%.2f\n", a->element->id_usine, valeur);
        }
        
        ecrire_resultats(a->fd, flux, mode);
    }
}
