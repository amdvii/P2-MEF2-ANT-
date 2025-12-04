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
