#include "fichier.h"
#include <string.h>

// --- Fonctions utilitaires ---

int max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

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
        // L'élément existe déjà. 
        // Dans ce projet, on n'écrase pas l'élément ici car on met à jour les sommes
        // via la fonction de recherche avant l'insertion.
        // On libère la structure temporaire qui ne sera pas insérée.
        free(e);
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

void traiter_fichier(const char* nom_fichier, AVL** arbre, int* h) {
    FILE* file = fopen(nom_fichier, "r");
    if (file == NULL) {
        printf("Erreur d'ouverture du fichier\n");
        exit(1);
    }

    char ligne[MAX_LINE_SIZE];
    
    // Lecture ligne par ligne
    while (fgets(ligne, MAX_LINE_SIZE, file) != NULL) {
        
        // --- Parsing Manuel (car strtok saute les champs vides ;;) ---
        char *ptr = ligne;
        char *col[6]; // Tableau pour stocker les pointeurs vers les 5 colonnes
        int i = 0;

        col[0] = ptr; // 1ere colonne
        while (*ptr) {
            if (*ptr == ';') {
                *ptr = '\0'; // On remplace le ; par une fin de chaine
                if (i < 5) {
                    col[++i] = ptr + 1; // La colonne suivante commence apres
                }
            } else if (*ptr == '\n' || *ptr == '\r') {
                *ptr = '\0'; // On nettoie la fin de ligne
            }
            ptr++;
        }
        
        // Sécurité : si la ligne est mal formée
        if (i < 4) continue; 

        // Les colonnes selon le PDF :
        // col[1] = Identifiant Amont (Départ)
        // col[2] = Identifiant Aval (Arrivée)
        // col[3] = Volume (Capacité ou Captage)
        // col[4] = Fuite %

        // Filtre : On ne s'intéresse qu'aux lignes qui ont un Volume défini.
        // Si col[3] est vide ou "-", c'est du transport/distribution => on ignore.
        if (strcmp(col[3], "-") == 0 || strlen(col[3]) == 0) {
            continue;
        }

        // CAS 1 : Définition d'une USINE (Capacité)
        // C'est le cas si la destination (col 2) est "-" (vide)
        if (strcmp(col[2], "-") == 0) {
            char* id_usine = col[1];
            double capacite = atof(col[3]);

            AVL* noeud = rechercherAVL(*arbre, id_usine);
            
            if (noeud == NULL) {
                // Nouvelle usine trouvée
                Usine_donnees* u = calloc(1, sizeof(Usine_donnees)); // calloc initialise à 0
                strcpy(u->id_usine, id_usine);
                u->capacite_max = capacite;
                *arbre = insertionAVL(*arbre, u, h);
            } else {
                // Usine existante (déjà créée par un captage), on ajoute la capacité
                noeud->element->capacite_max = capacite;
            }
        }
        
        // CAS 2 : CAPTAGE (Source -> Usine)
        // C'est le cas si la destination (col 2) n'est PAS "-"
        // Et on sait déjà que col[3] (Volume) n'est pas "-" grâce au filtre au début.
        else {
            char* id_usine = col[2]; // L'usine est en aval (destination)
            double volume_capte = atof(col[3]);
            double fuite_percent = 0.0;

            if (strcmp(col[4], "-") != 0 && strlen(col[4]) > 0) {
                fuite_percent = atof(col[4]);
            }

            AVL* noeud = rechercherAVL(*arbre, id_usine);

            if (noeud == NULL) {
                // On découvre l'usine via son captage
                Usine_donnees* u = calloc(1, sizeof(Usine_donnees));
                strcpy(u->id_usine, id_usine);
                
                u->volume_source = volume_capte;
                // Calcul du volume traité : Vol * (1 - fuite%)
                u->volume_traite = volume_capte * (1.0 - (fuite_percent / 100.0));
                
                *arbre = insertionAVL(*arbre, u, h);
            } else {
                // On met à jour les sommes
                noeud->element->volume_source += volume_capte;
                noeud->element->volume_traite += (volume_capte * (1.0 - (fuite_percent / 100.0)));
            }
        }
    }

    fclose(file);
}

void ecrire_resultats(AVL* a, FILE* flux) {
    if (a != NULL) {
        // Parcours infixe (tri alphabétique croissant, mais le shell triera numériquement)
        ecrire_resultats(a->fg, flux);
        
        // On n'écrit que si on a des données pertinentes (au moins une capacité ou une source)
        if (a->element->capacite_max > 0 || a->element->volume_source > 0) {
            // Format de sortie pour le bonus : ID;Capacite;Source;Traite
            // %.0f pour éviter les virgules inutiles sur les gros volumes
            fprintf(flux, "%s;%.0f;%.0f;%.0f\n", 
                    a->element->id_usine, 
                    a->element->capacite_max, 
                    a->element->volume_source, 
                    a->element->volume_traite);
        }
        
        ecrire_resultats(a->fd, flux);
    }
}