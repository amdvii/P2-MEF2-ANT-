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
        // On découpe la ligne
        char* col1 = strtok(ligne, ";"); // ID Usine (pour info, ou vide)
        (void)col1; // On ne s'en sert pas pour l'identification du type de ligne

        char* col2 = strtok(NULL, ";"); // Identifiant Amont (ou ID Usine si def)
        char* col3 = strtok(NULL, ";"); // Identifiant Aval (ou "-" si def)
        char* col4 = strtok(NULL, ";"); // Volume/Capacité (ou "-" si aval)
        char* col5 = strtok(NULL, ";"); // Rendement

        // Sécurité si ligne incomplète
        if (!col2 || !col3 || !col4) continue;

        // --- CAS 1 : Définition d'une usine (Capacité Max) ---
        // Le PDF dit : Col 3 est "-" ET Col 4 contient la capacité (donc pas "-")
        if (strcmp(col3, "-") == 0 && strcmp(col4, "-") != 0) {
             // Ici, col2 est l'identifiant de l'usine
             AVL* noeud = rechercherAVL(*arbre, col2);
             if (noeud == NULL) {
                 // Nouvelle usine trouvée
                 Usine_donnees* u = malloc(sizeof(Usine_donnees));
                 strcpy(u->id_usine, col2);
                 u->capacite_max = atof(col4);
                 u->volume_source = 0;
                 u->volume_traite = 0;
                 *arbre = insertionAVL(*arbre, u, h);
             } else {
                 // Usine déjà connue (via une source), on met à jour sa capacité
                 noeud->element->capacite_max = atof(col4);
             }
        }

        // --- CAS 2 : Liaison Source -> Usine (Volume capté) ---
        // Le PDF dit : Col 3 est l'usine, Col 4 est le volume (donc pas "-")
        // Note : Les lignes vers Stockage/Jonction ont "-" en Col 4, donc elles sont exclues ici.
        else if (strcmp(col3, "-") != 0 && strcmp(col4, "-") != 0) {
            // Ici, col3 est l'identifiant de l'usine (destination de la source)
            char* id_usine = col3; 
            double volume = atof(col4);
            (void)col5; // Pour éviter le warning

            AVL* noeud = rechercherAVL(*arbre, id_usine);
            if (noeud == NULL) {
                // On découvre l'usine via la source
                Usine_donnees* u = malloc(sizeof(Usine_donnees));
                strcpy(u->id_usine, id_usine);
                u->capacite_max = 0; // On ne la connait pas encore
                u->volume_source = volume;
                u->volume_traite = 0;
                *arbre = insertionAVL(*arbre, u, h);
            } else {
                // On ajoute le volume à l'usine existante
                noeud->element->volume_source += volume;
            }
        }
    }
    fclose(file);
}

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
