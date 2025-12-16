#ifndef FICHIER_H
#define FICHIER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* -------- Constantes -------- */
#define MAX_LIGNE 2048
#define MAX_ID    128

/* -------- Modes histo -------- */
typedef enum {
    HISTO_MAX = 0,
    HISTO_SRC = 1,
    HISTO_REAL = 2
} ModeHisto;

/* -------- Données usine (AVL) -------- */
typedef struct {
    char id_usine[MAX_ID];
    double capacite_max_km3;   /* col 4 des lignes USINE (en milliers de m3/an) */
    double volume_src_km3;     /* somme des captages (source->usine) en milliers de m3/an */
    double volume_real_km3;    /* somme pondérée (src*(1 - fuite%)) en milliers de m3/an */
} UsineDonnees;

typedef struct avl_usine {
    UsineDonnees *u;
    struct avl_usine *fg;
    struct avl_usine *fd;
    int eq;
} AVLUsine;

/* -------- Graphe aval (leaks) -------- */
typedef struct Noeud Noeud;

typedef struct Enfant {
    Noeud *child;
    double fuite_pct;          /* pourcentage de fuites sur le tronçon */
    struct Enfant *next;
} Enfant;

struct Noeud {
    char id[MAX_ID];
    Enfant *enfants;
    int deg;
};

typedef struct avl_noeud {
    Noeud *n;
    struct avl_noeud *fg;
    struct avl_noeud *fd;
    int eq;
} AVLNoeud;

/* -------- API principale -------- */
int traiter_histo(const char *chemin_fichier, ModeHisto mode, const char *out_dat);
int traiter_leaks(const char *chemin_fichier, const char *id_usine, const char *out_dat);

/* -------- Utilitaires -------- */
void nettoyer_fin_ligne(char *s);
int split_5_colonnes(char *ligne, char *col[5]); /* parsing robuste ; avec champs vides */
int est_tiret_ou_vide(const char *s);
double atof_safe(const char *s, int *ok);

/* -------- AVL usine -------- */
AVLUsine* avlU_inserer(AVLUsine *a, UsineDonnees *u, int *h);
AVLUsine* avlU_rechercher(AVLUsine *a, const char *id);
void avlU_liberer(AVLUsine *a);
void avlU_ecrire_inverse(AVLUsine *a, FILE *f, ModeHisto mode);

/* -------- AVL noeud -------- */
AVLNoeud* avlN_inserer(AVLNoeud *a, Noeud *n, int *h);
AVLNoeud* avlN_rechercher(AVLNoeud *a, const char *id);
void avlN_liberer(AVLNoeud *a);

/* -------- Graphe aval -------- */
Noeud* get_or_create_noeud(AVLNoeud **index, const char *id);
void ajouter_arete(Noeud *parent, Noeud *child, double fuite_pct);
void liberer_graphe(AVLNoeud *index);

#endif
