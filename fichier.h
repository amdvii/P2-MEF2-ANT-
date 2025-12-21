#ifndef FICHIER_H
#define FICHIER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------- Constantes -------- */
#define MAX_LIGNE 4096
#define MAX_ID    128

/* -------- Modes histo -------- */
typedef enum {
    HISTO_MAX  = 0,
    HISTO_SRC  = 1,
    HISTO_REAL = 2,
    HISTO_ALL  = 3   /* BONUS */
} ModeHisto;

/* -------- Données usine (AVL) --------
 * On stocke les 3 volumes demandés par le sujet :
 *  - max  : capacité max (usine)
 *  - src  : somme des volumes sources->usine
 *  - real : somme des volumes avec pertes (fuite%)
 */
typedef struct {
    char id_usine[MAX_ID];
    double capacite_max_km3;   /* col 4 des lignes USINE (en milliers de m3/an) */
    double volume_src_km3;     /* somme des captages (source->usine) en milliers de m3/an */
    double volume_real_km3;    /* somme pondérée : src*(1 - fuite%) en milliers de m3/an */
} UsineDonnees;

typedef struct avl_usine {
    UsineDonnees *u;
    struct avl_usine *fg;
    struct avl_usine *fd;
    int eq;
} AVLUsine;

/* -------- Graphe aval (leaks) --------
 * On construit un graphe orienté amont -> aval avec un taux de fuite par arête.
 */
typedef struct Noeud Noeud;

typedef struct Enfant {
    Noeud *child;              /* noeud aval */
    double fuite_pct;          /* pourcentage de fuites sur le tronçon */
    struct Enfant *next;
} Enfant;

struct Noeud {
    char id[MAX_ID];
    Enfant *enfants;
    int deg;                   /* nombre de sorties (enfants) */
};

typedef struct avl_noeud {
    Noeud *n;
    struct avl_noeud *fg;
    struct avl_noeud *fd;
    int eq;
} AVLNoeud;

/* -------- Pile (propagation leaks) --------
 * On propage un débit dans le graphe : chaque élément contient un noeud + le débit qui arrive.
 */
typedef struct {
    Noeud *n;
    double debit;
} ElementPile;

/* -------- API principale -------- */
int traiter_histo(const char *chemin_fichier, ModeHisto mode, const char *out_dat);
int traiter_leaks(const char *chemin_fichier, const char *id_usine, const char *out_dat);

/* -------- Utilitaires -------- */
void nettoyer_fin_ligne(char *s);
int split_5_colonnes(char *ligne, char *col[5]);
int est_tiret_ou_vide(const char *s);
double atof_safe(const char *s, int *ok);
int lire_ligne(FILE *f, char *buf, int taille);
int max2(int a, int b);
int max0(int x);
int min0(int x);

/* -------- AVL usine -------- */
AVLUsine* avlU_creer(UsineDonnees *u);
AVLUsine* rotG_U(AVLUsine *a);
AVLUsine* rotD_U(AVLUsine *a);
AVLUsine* rotGD_U(AVLUsine *a);
AVLUsine* rotDG_U(AVLUsine *a);
AVLUsine* equilibrer_U(AVLUsine *a);
AVLUsine* avlU_inserer(AVLUsine *a, UsineDonnees *u, int *h, int *err);
AVLUsine* avlU_rechercher(AVLUsine *a, const char *id);
void avlU_liberer(AVLUsine *a);
void avlU_ecrire_inverse(AVLUsine *a, FILE *f, ModeHisto mode);

/* -------- AVL noeud -------- */
AVLNoeud* avlN_creer(Noeud *n);
AVLNoeud* rotG_N(AVLNoeud *a);
AVLNoeud* rotD_N(AVLNoeud *a);
AVLNoeud* rotGD_N(AVLNoeud *a);
AVLNoeud* rotDG_N(AVLNoeud *a);
AVLNoeud* equilibrer_N(AVLNoeud *a);
AVLNoeud* avlN_inserer(AVLNoeud *a, Noeud *n, int *h, int *err);
AVLNoeud* avlN_rechercher(AVLNoeud *a, const char *id);
void liberer_enfants(Enfant *e);
void avlN_liberer(AVLNoeud *a);

/* -------- Graphe aval -------- */
Noeud* obtenir_ou_creer_noeud(AVLNoeud **index, const char *id);
void ajouter_arete(Noeud *parent, Noeud *child, double fuite_pct);
void liberer_graphe(AVLNoeud *index);

/* -------- HISTO -------- */
const char* entete_histo(ModeHisto mode);

/* -------- LEAKS -------- */
int fichier_vide_ou_absent(const char *path);
void construire_chemin_leaks_bonus(const char *out_dat, char *out_bonus, int taille);

#endif
