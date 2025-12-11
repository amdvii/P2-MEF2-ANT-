#include "fichier.h"
#include <string.h>

// --- Fonctions utilitaires ---

// Fonction simple pour le maximum
int max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

// Fonction simple pour le minimum
int min(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
}

// --- Fonctions AVL ---

AVL* creerArbre(Usine_donnees* data) {
    AVL* nouveau = malloc(sizeof(AVL));
    if (nouveau == NULL) {
        exit(1);
    }
    nouveau->element = data;
    nouveau->fg = NULL;
    nouveau->fd = NULL;
    nouveau->equilibre = 0;
    return nouveau;
}

// Rotation simple gauche
AVL* rotationGauche(AVL* a) {
    AVL* pivot;
    int eq_a, eq_p;

    pivot = a->fd;
    a->fd = pivot->fg;
    pivot->fg = a;

    eq_a = a->equilibre;
    eq_p = pivot->equilibre;

    a->equilibre = eq_a - 1 - max(eq_p, 0);
    pivot->equilibre = eq_p - 1 + min(a->equilibre, 0);

    return pivot;
}

// Rotation simple droite
AVL* rotationDroite(AVL* a) {
    AVL* pivot;
    int eq_a, eq_p;

    pivot = a->fg;
    a->fg = pivot->fd;
    pivot->fd = a;

    eq_a = a->equilibre;
    eq_p = pivot->equilibre;

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
    } else if (a->equilibre <= -2) {
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
    } else if (cmp > 0) {
        a->fd = insertionAVL(a->fd, e, h);
    } else {
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

// --- Traitement du fichier ---

void traiter_fichier(const char* nom_fichier, char* filtre_station, char* filtre_conso, AVL** arbre, int* h) {
    FILE* file = fopen(nom_fichier, "r");
    if (file == NULL) {
        printf("Erreur d'ouverture du fichier\n");
        exit(1);
    }

    char ligne[1024];
    char col1[50], col2[50], col3[50], col4[50];
    
    // On ignore l'entête si besoin, ou on lit directement
    // Ici on lit ligne par ligne avec fgets
    while (fgets(ligne, 1024, file) != NULL) {
        
        // Utilisation de sscanf pour récupérer les champs séparés par ';'
        // %[^;] veut dire : lire tout jusqu'au point-virgule
        int nb_lus = sscanf(ligne, "%[^;];%[^;];%[^;];%[^;]", col1, col2, col3, col4);

        if (nb_lus < 4) {
            continue; 
        }

        // On enlève le filtre sur col1 pour l'instant car ton fichier commence par "-"
        // C'est ici qu'on rétablira le filtre plus tard si le format change

        // Cas 1 : Définition d'une station (col3 est un tiret "-")
        if (strcmp(col3, "-") == 0) {
            AVL* noeud = rechercherAVL(*arbre, col2);
            
            if (noeud == NULL) {
                // Création d'une nouvelle structure
                Usine_donnees* u = malloc(sizeof(Usine_donnees));
                strcpy(u->id_usine, col2);
                u->capacite_max = atof(col4); // Conversion string -> double
                u->volume_source = 0;
                
                *arbre = insertionAVL(*arbre, u, h);
            } else {
                // Mise à jour si elle existe déjà
                noeud->element->capacite_max = atof(col4);
            }
        }
        // Cas 2 : Liaison (col3 n'est pas un tiret)
        else {
            AVL* noeud = rechercherAVL(*arbre, col2);
            
            if (noeud == NULL) {
                Usine_donnees* u = malloc(sizeof(Usine_donnees));
                strcpy(u->id_usine, col2);
                u->capacite_max = 0;
                u->volume_source = atof(col4);
                
                *arbre = insertionAVL(*arbre, u, h);
            } else {
                noeud->element->volume_source = noeud->element->volume_source + atof(col4);
            }
        }
    }

    fclose(file);
}

void ecrire_resultats(AVL* a, FILE* flux, const char* mode) {
    if (a != NULL) {
        ecrire_resultats(a->fg, flux, mode);
        
        double valeur = 0;
        
        // On prend la valeur la plus pertinente
        if (a->element->capacite_max > 0) {
            valeur = a->element->capacite_max;
        } else {
            valeur = a->element->volume_source;
        }

        // On écrit dans le fichier si la valeur est positive
        if (valeur > 0) {
            fprintf(flux, "%s;%.2f\n", a->element->id_usine, valeur);
        }
        
        ecrire_resultats(a->fd, flux, mode);
    }
}