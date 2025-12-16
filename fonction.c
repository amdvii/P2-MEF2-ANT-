#include "fichier.h"

/* ============================================================
   Utilitaires
   ============================================================ */

void nettoyer_fin_ligne(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

int est_tiret_ou_vide(const char *s) {
    if (s == NULL) return 1;
    if (s[0] == '\0') return 1;
    return (strcmp(s, "-") == 0);
}

double atof_safe(const char *s, int *ok) {
    char *end = NULL;
    errno = 0;
    double v = strtod(s, &end);
    if (errno != 0 || end == s) {
        if (ok) *ok = 0;
        return 0.0;
    }
    if (ok) *ok = 1;
    return v;
}

/* Parsing manuel 5 colonnes ; accepte champs vides ; remplace ; par \0 */
int split_5_colonnes(char *ligne, char *col[5]) {
    if (!ligne) return 0;
    nettoyer_fin_ligne(ligne);

    for (int i = 0; i < 5; i++) col[i] = NULL;
    col[0] = ligne;

    int c = 1;
    for (char *p = ligne; *p; p++) {
        if (*p == ';') {
            *p = '\0';
            if (c < 5) col[c++] = p + 1;
        }
    }

    /* si moins de 5 colonnes, on complète par "" */
    for (int i = 0; i < 5; i++) {
        if (col[i] == NULL) col[i] = (char*)"";
    }
    return 1;
}

/* ============================================================
   AVL Usine (par id_usine)
   ============================================================ */

static int max2(int a, int b) { return (a > b) ? a : b; }

static AVLUsine* avlU_creer(UsineDonnees *u) {
    AVLUsine *n = (AVLUsine*)malloc(sizeof(AVLUsine));
    if (!n) return NULL;
    n->u = u;
    n->fg = n->fd = NULL;
    n->eq = 0;
    return n;
}

static AVLUsine* rotG_U(AVLUsine *a) {
    AVLUsine *p = a->fd;
    int eq_a = a->eq, eq_p = p->eq;

    a->fd = p->fg;
    p->fg = a;

    a->eq = eq_a - 1 - max2(eq_p, 0);
    p->eq = eq_p - 1 + ((a->eq < 0) ? a->eq : 0);
    return p;
}

static AVLUsine* rotD_U(AVLUsine *a) {
    AVLUsine *p = a->fg;
    int eq_a = a->eq, eq_p = p->eq;

    a->fg = p->fd;
    p->fd = a;

    a->eq = eq_a + 1 - ((eq_p > 0) ? eq_p : 0);
    p->eq = eq_p + 1 + max2(a->eq, 0);
    return p;
}

static AVLUsine* rotGD_U(AVLUsine *a) { a->fd = rotD_U(a->fd); return rotG_U(a); }
static AVLUsine* rotDG_U(AVLUsine *a) { a->fg = rotG_U(a->fg); return rotD_U(a); }

static AVLUsine* equilibrer_U(AVLUsine *a) {
    if (a->eq >= 2) {
        if (a->fd && a->fd->eq >= 0) return rotG_U(a);
        return rotGD_U(a);
    }
    if (a->eq <= -2) {
        if (a->fg && a->fg->eq <= 0) return rotD_U(a);
        return rotDG_U(a);
    }
    return a;
}

AVLUsine* avlU_inserer(AVLUsine *a, UsineDonnees *u, int *h) {
    if (!a) { *h = 1; return avlU_creer(u); }

    int cmp = strcmp(u->id_usine, a->u->id_usine);
    if (cmp < 0) {
        a->fg = avlU_inserer(a->fg, u, h);
        *h = -(*h);
    } else if (cmp > 0) {
        a->fd = avlU_inserer(a->fd, u, h);
    } else {
        /* déjà présent : on libère la nouvelle structure */
        free(u);
        *h = 0;
        return a;
    }

    if (*h != 0) {
        a->eq += *h;
        if (a->eq == 0) *h = 0;
        else *h = 1;
    }

    if (a->eq >= 2 || a->eq <= -2) {
        a = equilibrer_U(a);
        *h = 0;
    }
    return a;
}

AVLUsine* avlU_rechercher(AVLUsine *a, const char *id) {
    if (!a) return NULL;
    int cmp = strcmp(id, a->u->id_usine);
    if (cmp == 0) return a;
    if (cmp < 0) return avlU_rechercher(a->fg, id);
    return avlU_rechercher(a->fd, id);
}

void avlU_liberer(AVLUsine *a) {
    if (!a) return;
    avlU_liberer(a->fg);
    avlU_liberer(a->fd);
    free(a->u);
    free(a);
}

/* écriture inverse : fd -> noeud -> fg  (ordre alphabétique inverse) */
void avlU_ecrire_inverse(AVLUsine *a, FILE *f, ModeHisto mode) {
    if (!a) return;
    avlU_ecrire_inverse(a->fd, f, mode);

    double km3 = 0.0;
    if (mode == HISTO_MAX)  km3 = a->u->capacite_max_km3;
    if (mode == HISTO_SRC)  km3 = a->u->volume_src_km3;
    if (mode == HISTO_REAL) km3 = a->u->volume_real_km3;

    /* sortie demandée en millions de m3 : km3 / 1000 */
    double Mm3 = km3 / 1000.0;
    fprintf(f, "%s;%.6f\n", a->u->id_usine, Mm3);

    avlU_ecrire_inverse(a->fg, f, mode);
}

/* ============================================================
   AVL Noeud (index pour leaks)
   ============================================================ */

static AVLNoeud* avlN_creer(Noeud *n) {
    AVLNoeud *x = (AVLNoeud*)malloc(sizeof(AVLNoeud));
    if (!x) return NULL;
    x->n = n;
    x->fg = x->fd = NULL;
    x->eq = 0;
    return x;
}

static AVLNoeud* rotG_N(AVLNoeud *a) {
    AVLNoeud *p = a->fd;
    int eq_a = a->eq, eq_p = p->eq;

    a->fd = p->fg;
    p->fg = a;

    a->eq = eq_a - 1 - max2(eq_p, 0);
    p->eq = eq_p - 1 + ((a->eq < 0) ? a->eq : 0);
    return p;
}

static AVLNoeud* rotD_N(AVLNoeud *a) {
    AVLNoeud *p = a->fg;
    int eq_a = a->eq, eq_p = p->eq;

    a->fg = p->fd;
    p->fd = a;

    a->eq = eq_a + 1 - ((eq_p > 0) ? eq_p : 0);
    p->eq = eq_p + 1 + max2(a->eq, 0);
    return p;
}

static AVLNoeud* rotGD_N(AVLNoeud *a) { a->fd = rotD_N(a->fd); return rotG_N(a); }
static AVLNoeud* rotDG_N(AVLNoeud *a) { a->fg = rotG_N(a->fg); return rotD_N(a); }

static AVLNoeud* equilibrer_N(AVLNoeud *a) {
    if (a->eq >= 2) {
        if (a->fd && a->fd->eq >= 0) return rotG_N(a);
        return rotGD_N(a);
    }
    if (a->eq <= -2) {
        if (a->fg && a->fg->eq <= 0) return rotD_N(a);
        return rotDG_N(a);
    }
    return a;
}

AVLNoeud* avlN_inserer(AVLNoeud *a, Noeud *n, int *h) {
    if (!a) { *h = 1; return avlN_creer(n); }

    int cmp = strcmp(n->id, a->n->id);
    if (cmp < 0) {
        a->fg = avlN_inserer(a->fg, n, h);
        *h = -(*h);
    } else if (cmp > 0) {
        a->fd = avlN_inserer(a->fd, n, h);
    } else {
        /* déjà là : libérer le nouveau noeud */
        free(n);
        *h = 0;
        return a;
    }

    if (*h != 0) {
        a->eq += *h;
        if (a->eq == 0) *h = 0;
        else *h = 1;
    }

    if (a->eq >= 2 || a->eq <= -2) {
        a = equilibrer_N(a);
        *h = 0;
    }
    return a;
}

AVLNoeud* avlN_rechercher(AVLNoeud *a, const char *id) {
    if (!a) return NULL;
    int cmp = strcmp(id, a->n->id);
    if (cmp == 0) return a;
    if (cmp < 0) return avlN_rechercher(a->fg, id);
    return avlN_rechercher(a->fd, id);
}

static void free_enfants(Enfant *e) {
    while (e) {
        Enfant *n = e->next;
        free(e);
        e = n;
    }
}

void avlN_liberer(AVLNoeud *a) {
    if (!a) return;
    avlN_liberer(a->fg);
    avlN_liberer(a->fd);
    free_enfants(a->n->enfants);
    free(a->n);
    free(a);
}

/* ============================================================
   Graphe aval (leaks)
   ============================================================ */

Noeud* get_or_create_noeud(AVLNoeud **index, const char *id) {
    AVLNoeud *found = avlN_rechercher(*index, id);
    if (found) return found->n;

    Noeud *n = (Noeud*)calloc(1, sizeof(Noeud));
    if (!n) return NULL;
    strncpy(n->id, id, MAX_ID - 1);
    n->id[MAX_ID - 1] = '\0';
    n->enfants = NULL;
    n->deg = 0;

    int h = 0;
    *index = avlN_inserer(*index, n, &h);
    /* si insertion échoue par manque mémoire -> n peut être free dans avlN_inserer,
       ici on ne gère pas finement : le programme retournera une erreur plus loin */
    return (avlN_rechercher(*index, id)) ? avlN_rechercher(*index, id)->n : NULL;
}

void ajouter_arete(Noeud *parent, Noeud *child, double fuite_pct) {
    if (!parent || !child) return;
    Enfant *e = (Enfant*)malloc(sizeof(Enfant));
    if (!e) return;
    e->child = child;
    e->fuite_pct = fuite_pct;
    e->next = parent->enfants;
    parent->enfants = e;
    parent->deg += 1;
}

void liberer_graphe(AVLNoeud *index) {
    avlN_liberer(index);
}

/* ============================================================
   Traitement HISTO
   ============================================================ */

static const char* header_histo(ModeHisto mode) {
    if (mode == HISTO_MAX)  return "identifier;max volume (M.m3.year-1)\n";
    if (mode == HISTO_SRC)  return "identifier;source volume (M.m3.year-1)\n";
    return "identifier;real volume (M.m3.year-1)\n";
}

/* Lit le fichier et remplit l'AVL usine :
   - lignes USINE : col[1]=usine, col[3]=capacité
   - lignes SOURCE->USINE : col[2]=usine, col[3]=volume capté, col[4]=fuite%
*/
int traiter_histo(const char *chemin_fichier, ModeHisto mode, const char *out_dat) {
    FILE *in = fopen(chemin_fichier, "r");
    if (!in) {
        fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", chemin_fichier);
        return 10;
    }

    AVLUsine *arbre = NULL;
    int h = 0;

    char ligne[MAX_LIGNE];
    while (fgets(ligne, sizeof(ligne), in)) {
        char *col[5];
        if (!split_5_colonnes(ligne, col)) continue;

        /* Colonne #2 = amont (col[1]), #3 = aval (col[2]), #4 = volume (col[3]), #5 = fuite (col[4]) */
        if (est_tiret_ou_vide(col[3])) {
            /* pas de volume => pas utile pour histo (distribution aval) */
            continue;
        }

        /* CAS USINE: exemple "-;Facility...;-;4749292;-"
           => aval (col[2]) == "-" et amont (col[1]) = id usine */
        if (est_tiret_ou_vide(col[2])) {
            const char *id_usine = col[1];
            int ok = 0;
            double cap = atof_safe(col[3], &ok);
            if (!ok) continue;

            AVLUsine *node = avlU_rechercher(arbre, id_usine);
            if (!node) {
                UsineDonnees *u = (UsineDonnees*)calloc(1, sizeof(UsineDonnees));
                if (!u) { fclose(in); avlU_liberer(arbre); return 11; }
                strncpy(u->id_usine, id_usine, MAX_ID-1);
                u->capacite_max_km3 = cap;
                arbre = avlU_inserer(arbre, u, &h);
            } else {
                node->u->capacite_max_km3 = cap;
            }
        } else {
            /* CAS SOURCE->USINE: aval = usine */
            const char *id_usine = col[2];
            int okV = 0;
            double vol = atof_safe(col[3], &okV);
            if (!okV) continue;

            double fuite = 0.0;
            if (!est_tiret_ou_vide(col[4])) {
                int okF = 0;
                fuite = atof_safe(col[4], &okF);
                if (!okF) fuite = 0.0;
            }

            AVLUsine *node = avlU_rechercher(arbre, id_usine);
            if (!node) {
                UsineDonnees *u = (UsineDonnees*)calloc(1, sizeof(UsineDonnees));
                if (!u) { fclose(in); avlU_liberer(arbre); return 12; }
                strncpy(u->id_usine, id_usine, MAX_ID-1);
                arbre = avlU_inserer(arbre, u, &h);
                node = avlU_rechercher(arbre, id_usine);
                if (!node) { fclose(in); avlU_liberer(arbre); return 13; }
            }

            node->u->volume_src_km3 += vol;
            node->u->volume_real_km3 += vol * (1.0 - (fuite / 100.0));
        }
    }

    fclose(in);

    FILE *out = fopen(out_dat, "w");
    if (!out) {
        fprintf(stderr, "Erreur: impossible de créer %s\n", out_dat);
        avlU_liberer(arbre);
        return 20;
    }

    fputs(header_histo(mode), out);
    avlU_ecrire_inverse(arbre, out, mode);
    fclose(out);

    avlU_liberer(arbre);
    return 0;
}

/* ============================================================
   Traitement LEAKS
   ============================================================ */

/* 1) calcule volume real (Mm3/an) de l'usine via source->usine
   2) construit graphe aval (enfants) pour cette usine
   3) propage débit, somme pertes
   4) append dans out_dat (création si absent)
*/
int traiter_leaks(const char *chemin_fichier, const char *id_usine, const char *out_dat) {
    /* --- étape 1 : récupérer volume "real" de l'usine (km3) + vérifier existence usine --- */
    FILE *in = fopen(chemin_fichier, "r");
    if (!in) {
        fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", chemin_fichier);
        return 30;
    }

    int usine_trouvee = 0;
    double real_km3 = 0.0;

    char ligne[MAX_LIGNE];
    while (fgets(ligne, sizeof(ligne), in)) {
        char *col[5];
        if (!split_5_colonnes(ligne, col)) continue;

        /* USINE line => col[1] = usine et col[2] = "-" */
        if (est_tiret_ou_vide(col[2]) && !est_tiret_ou_vide(col[1])) {
            if (strcmp(col[1], id_usine) == 0) {
                usine_trouvee = 1;
            }
            continue;
        }

        /* source->usine : col[2] = usine, col[3]=vol, col[4]=fuite */
        if (!est_tiret_ou_vide(col[2]) && !est_tiret_ou_vide(col[3])) {
            if (strcmp(col[2], id_usine) == 0) {
                int okV = 0;
                double vol = atof_safe(col[3], &okV);
                if (!okV) continue;

                double fuite = 0.0;
                if (!est_tiret_ou_vide(col[4])) {
                    int okF = 0;
                    fuite = atof_safe(col[4], &okF);
                    if (!okF) fuite = 0.0;
                }
                real_km3 += vol * (1.0 - (fuite / 100.0));
            }
        }
    }
    fclose(in);

    /* si l'usine n'est pas dans le fichier, résultat -1 */
    double pertes_Mm3 = -1.0;
    if (!usine_trouvee) {
        FILE *out = fopen(out_dat, "a");
        if (!out) {
            /* si fichier absent/permission, on tente de le créer avec header */
            out = fopen(out_dat, "w");
            if (!out) return 31;
            fputs("identifier;Leak volume (M.m3.year-1)\n", out);
        } else {
            /* si c'est un fichier vide, ajouter header */
            long pos = ftell(out);
            if (pos == 0) fputs("identifier;Leak volume (M.m3.year-1)\n", out);
        }
        fprintf(out, "%s;%.6f\n", id_usine, pertes_Mm3);
        fclose(out);
        return 0;
    }

    /* --- étape 2 : construire le graphe aval pour cette usine --- */
    AVLNoeud *index = NULL;

    Noeud *racine = get_or_create_noeud(&index, id_usine);
    if (!racine) { liberer_graphe(index); return 32; }

    in = fopen(chemin_fichier, "r");
    if (!in) { liberer_graphe(index); return 33; }

    while (fgets(ligne, sizeof(ligne), in)) {
        char *col[5];
        if (!split_5_colonnes(ligne, col)) continue;

        /* Pour le réseau aval, les lignes utiles sont :
           - USINE -> STOCKAGE : "-;Facility;Storage;-;fuite"
             => parent = col[1] (usine), child=col[2]
           - puis toutes les distributions : col[0] == id_usine (eau traitée par cette usine),
             parent = col[1], child = col[2], fuite = col[4]
           On ignore source->usine (col[0] == "-" et parent = Spring)
        */

        const char *treated_by = col[0];
        const char *amont = col[1];
        const char *aval  = col[2];

        /* ligne USINE -> STOCKAGE : amont == id_usine et aval commence par "Storage " en général */
        int est_usine_stockage = (strcmp(amont, id_usine) == 0) && !est_tiret_ou_vide(aval) && !est_tiret_ou_vide(col[4]);

        /* lignes aval : col[0] == id_usine */
        int est_aval_filtre = (strcmp(treated_by, id_usine) == 0) && !est_tiret_ou_vide(amont) && !est_tiret_ou_vide(aval) && !est_tiret_ou_vide(col[4]);

        if (!est_usine_stockage && !est_aval_filtre) continue;

        int okF = 0;
        double fuite = atof_safe(col[4], &okF);
        if (!okF) fuite = 0.0;

        Noeud *p = get_or_create_noeud(&index, amont);
        Noeud *c = get_or_create_noeud(&index, aval);
        if (!p || !c) { fclose(in); liberer_graphe(index); return 34; }

        ajouter_arete(p, c, fuite);
    }
    fclose(in);

    /* --- étape 3 : propagation débit + somme pertes --- */
    /* débit de départ (Mm3) */
    double debit0 = real_km3 / 1000.0;

    typedef struct {
        Noeud *n;
        double debit;
    } StackItem;

    size_t cap = 1024, top = 0;
    StackItem *stack = (StackItem*)malloc(cap * sizeof(StackItem));
    if (!stack) { liberer_graphe(index); return 35; }

    stack[top++] = (StackItem){ racine, debit0 };

    double pertes = 0.0;

    while (top > 0) {
        StackItem it = stack[--top];
        Noeud *n = it.n;
        double debit = it.debit;

        if (!n || n->deg <= 0) continue;

        double part = debit / (double)n->deg;

        for (Enfant *e = n->enfants; e; e = e->next) {
            double f = e->fuite_pct / 100.0;
            if (f < 0.0) f = 0.0;
            if (f > 1.0) f = 1.0;

            double perdu = part * f;
            double restant = part - perdu;
            pertes += perdu;

            if (restant > 0.0 && e->child && e->child->deg > 0) {
                if (top >= cap) {
                    cap *= 2;
                    StackItem *tmp = (StackItem*)realloc(stack, cap * sizeof(StackItem));
                    if (!tmp) { free(stack); liberer_graphe(index); return 36; }
                    stack = tmp;
                }
                stack[top++] = (StackItem){ e->child, restant };
            }
        }
    }

    free(stack);
    liberer_graphe(index);

    pertes_Mm3 = pertes;

    /* --- étape 4 : écrire / compléter historique out_dat --- */
    FILE *out = fopen(out_dat, "a");
    if (!out) {
        out = fopen(out_dat, "w");
        if (!out) return 40;
        fputs("identifier;Leak volume (M.m3.year-1)\n", out);
    } else {
        long pos = ftell(out);
        if (pos == 0) fputs("identifier;Leak volume (M.m3.year-1)\n", out);
    }

    fprintf(out, "%s;%.6f\n", id_usine, pertes_Mm3);
    fclose(out);

    return 0;
}
