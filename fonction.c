#include "fichier.h"

/* -------- Utilitaires -------- */

/* Supprime \n / \r en fin de ligne. */
void nettoyer_fin_ligne(char *s) {
    int n;

    if (s == NULL) return;

    n = (int)strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

int est_tiret_ou_vide(const char *s) {
    if (s == NULL) return 1;
    if (s[0] == '\0') return 1;
    if (strcmp(s, "-") == 0) return 1;
    return 0;
}


/* Convertit une chaîne en double (retourne ok=0 si invalide). */
double atof_safe(const char *s, int *ok) {
    char *end;
    double v;

    if (ok) *ok = 0;
    if (s == NULL) return 0.0;
    if (est_tiret_ou_vide(s)) return 0.0;

    end = NULL;
    v = strtod(s, &end);
    if (end == s) return 0.0;

    while (*end == ' ' || *end == '\t') end++;
    if (*end != '\0') return 0.0;

    if (ok) *ok = 1;
    return v;
}

/* Découpe une ligne en 5 colonnes séparées par ';'.
   (On remplace les ';' par '\0' directement dans la chaîne.) */
int split_5_colonnes(char *ligne, char *col[5]) {
    int i;
    int c;
    char *p;

    if (ligne == NULL) return 0;
    nettoyer_fin_ligne(ligne);

    for (i = 0; i < 5; i++) col[i] = NULL;
    col[0] = ligne;

    c = 1;
    p = ligne;
    while (*p) {
        if (*p == ';') {
            *p = '\0';
            if (c < 5) col[c++] = p + 1;
        }
        p++;
    }

    for (i = 0; i < 5; i++) {
        if (col[i] == NULL) col[i] = (char*)"";
    }
    return 1;
}

/* Lit une ligne avec fgets.
   Si la ligne dépasse le buffer, on purge le reste jusqu'au '\n'. */
int lire_ligne(FILE *f, char *buf, int taille) {
    int ch;
    int len;

    if (fgets(buf, taille, f) == NULL) return 0;

    len = (int)strlen(buf);
    if (len > 0 && buf[len - 1] != '\n') {
        do {
            ch = fgetc(f);
        } while (ch != '\n' && ch != EOF);
    }
    return 1;
}

/* -------- Fonctions AVL (utilitaires) -------- */

int max2(int a, int b) {
    if (a > b) return a;
    return b;
}

int max0(int x) {
    if (x > 0) return x;
    return 0;
}

int min0(int x) {
    if (x < 0) return x;
    return 0;
}

/* -------- AVL Usine (tri par id) -------- */

AVLUsine* avlU_creer(UsineDonnees *u) {
    AVLUsine *n = malloc(sizeof(AVLUsine));
    if (!n) { return NULL; }
    n->u = u;
    n->fg = NULL;
    n->fd = NULL;
    n->eq = 0;
    return n;
}

AVLUsine* rotG_U(AVLUsine *a) {
    AVLUsine *p;
    int eq_a, eq_p;

    p = a->fd;
    eq_a = a->eq;
    eq_p = p->eq;

    a->fd = p->fg;
    p->fg = a;

    a->eq = eq_a - 1 - max2(eq_p, 0);
    p->eq = eq_p - 1 + min0(a->eq);
    return p;
}

AVLUsine* rotD_U(AVLUsine *a) {
    AVLUsine *p;
    int eq_a, eq_p;

    p = a->fg;
    eq_a = a->eq;
    eq_p = p->eq;

    a->fg = p->fd;
    p->fd = a;

    a->eq = eq_a + 1 - max0(eq_p);
    p->eq = eq_p + 1 + max0(a->eq);
    return p;
}

AVLUsine* rotGD_U(AVLUsine *a) {
    a->fd = rotD_U(a->fd);
    return rotG_U(a);
}

AVLUsine* rotDG_U(AVLUsine *a) {
    a->fg = rotG_U(a->fg);
    return rotD_U(a);
}

AVLUsine* equilibrer_U(AVLUsine *a) {
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

AVLUsine* avlU_inserer(AVLUsine *a, UsineDonnees *u, int *h, int *err) {
    int cmp;

    if (!h) return a;

    if (!a) {
        AVLUsine *nn = avlU_creer(u);
        if (!nn) {
            /* Échec d'allocation : on évite une fuite */
            free(u);
            *h = 0;
            return NULL;
        }
        *h = 1;
        return nn;
    }

    cmp = strcmp(u->id_usine, a->u->id_usine);
    if (cmp < 0) {
        AVLUsine *old = a->fg;
        AVLUsine *res = avlU_inserer(old, u, h, err);

        if (err && *err) {
            a->fg = old;
            *h = 0;
            return a;
        }

        a->fg = res;
        *h = -(*h);
    } else if (cmp > 0) {
        AVLUsine *old = a->fd;
        AVLUsine *res = avlU_inserer(old, u, h, err);

        if (err && *err) {
            a->fd = old;
            *h = 0;
            return a;
        }

        a->fd = res;
    } else {
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
    int cmp;

    if (!a) return NULL;
    cmp = strcmp(id, a->u->id_usine);
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

/* écriture en ordre alphabétique inverse */
void avlU_ecrire_inverse(AVLUsine *a, FILE *f, ModeHisto mode) {
    double max_Mm3, src_Mm3, real_Mm3, Mm3;

    if (!a) return;

    avlU_ecrire_inverse(a->fd, f, mode);

    if (mode == HISTO_ALL) {
        /*
         * HISTO_ALL : 3 valeurs 
         *  - bleu  : volume réellement fourni (real)
         *  - rouge : volume perdu après captage (src - real)
         *  - vert  : volume encore possible à traiter (max - src)
         */
        double bleu, rouge, vert;

        max_Mm3  = a->u->capacite_max_km3 / 1000.0;
        src_Mm3  = a->u->volume_src_km3   / 1000.0;
        real_Mm3 = a->u->volume_real_km3  / 1000.0;

        bleu  = real_Mm3;
        rouge = src_Mm3 - real_Mm3;
        vert  = max_Mm3 - src_Mm3;

        /* On évite les valeurs négatives dues à des dépassements ou à l'arrondi */
        if (bleu < 0.0) bleu = 0.0;
        if (rouge < 0.0) rouge = 0.0;
        if (vert < 0.0) vert = 0.0;

        fprintf(f, "%s;%.6f;%.6f;%.6f\n", a->u->id_usine, bleu, rouge, vert);
    } else {
        Mm3 = 0.0;
        if (mode == HISTO_MAX)  Mm3 = a->u->capacite_max_km3 / 1000.0;
        if (mode == HISTO_SRC)  Mm3 = a->u->volume_src_km3   / 1000.0;
        if (mode == HISTO_REAL) Mm3 = a->u->volume_real_km3  / 1000.0;

        fprintf(f, "%s;%.6f\n", a->u->id_usine, Mm3);
    }

    avlU_ecrire_inverse(a->fg, f, mode);
}

/* -------- AVL Noeud (index leaks) -------- */

AVLNoeud* avlN_creer(Noeud *n) {
    AVLNoeud *x = malloc(sizeof(AVLNoeud));
    if (!x) { return NULL; }
    x->n = n;
    x->fg = NULL;
    x->fd = NULL;
    x->eq = 0;
    return x;
}

AVLNoeud* rotG_N(AVLNoeud *a) {
    AVLNoeud *p;
    int eq_a, eq_p;

    p = a->fd;
    eq_a = a->eq;
    eq_p = p->eq;

    a->fd = p->fg;
    p->fg = a;

    a->eq = eq_a - 1 - max2(eq_p, 0);
    p->eq = eq_p - 1 + min0(a->eq);
    return p;
}

AVLNoeud* rotD_N(AVLNoeud *a) {
    AVLNoeud *p;
    int eq_a, eq_p;

    p = a->fg;
    eq_a = a->eq;
    eq_p = p->eq;

    a->fg = p->fd;
    p->fd = a;

    a->eq = eq_a + 1 - max0(eq_p);
    p->eq = eq_p + 1 + max0(a->eq);
    return p;
}

AVLNoeud* rotGD_N(AVLNoeud *a) {
    a->fd = rotD_N(a->fd);
    return rotG_N(a);
}

AVLNoeud* rotDG_N(AVLNoeud *a) {
    a->fg = rotG_N(a->fg);
    return rotD_N(a);
}

AVLNoeud* equilibrer_N(AVLNoeud *a) {
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

AVLNoeud* avlN_inserer(AVLNoeud *a, Noeud *n, int *h, int *err) {
    int cmp;

    if (!h) return a;

    if (!a) {
        AVLNoeud *nn = avlN_creer(n);
        if (!nn) {
            free(n);
            *h = 0;
            if (err) *err = 1;
            return NULL;
        }
        nn->n = n;
        *h = 1;
        return nn;
    }

    cmp = strcmp(n->id, a->n->id);
    if (cmp < 0) {
        AVLNoeud *old = a->fg;
        AVLNoeud *res = avlN_inserer(old, n, h, err);

        if (err && *err) {
            a->fg = old;
            *h = 0;
            return a;
        }

        a->fg = res;
        *h = -(*h);
    } else if (cmp > 0) {
        AVLNoeud *old = a->fd;
        AVLNoeud *res = avlN_inserer(old, n, h, err);

        if (err && *err) {
            a->fd = old;
            *h = 0;
            return a;
        }

        a->fd = res;
    } else {
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
    int cmp;

    if (!a) return NULL;
    cmp = strcmp(id, a->n->id);
    if (cmp == 0) return a;
    if (cmp < 0) return avlN_rechercher(a->fg, id);
    return avlN_rechercher(a->fd, id);
}

void liberer_enfants(Enfant *e) {
    Enfant *nxt;
    while (e) {
        nxt = e->next;
        free(e);
        e = nxt;
    }
}

void avlN_liberer(AVLNoeud *a) {
    if (!a) return;
    avlN_liberer(a->fg);
    avlN_liberer(a->fd);
    liberer_enfants(a->n->enfants);
    free(a->n);
    free(a);
}

/* -------- Graphe aval (leaks) -------- */

Noeud* obtenir_ou_creer_noeud(AVLNoeud **index, const char *id) {
    AVLNoeud *found;
    Noeud *n;
    int h;

    found = avlN_rechercher(*index, id);
    if (found) return found->n;

    n = malloc(sizeof(Noeud));
    if (!n) return NULL;

    strncpy(n->id, id, MAX_ID - 1);
    n->id[MAX_ID - 1] = '\0';
    n->enfants = NULL;
    n->deg = 0;    int err;
    AVLNoeud *newroot;

    err = 0;
    h = 0;
    newroot = avlN_inserer(*index, n, &h, &err);
    if (err) {
        /* avlN_inserer a déjà libéré n en cas d'échec */
        return NULL;
    }
    *index = newroot;
    found = avlN_rechercher(*index, id);
    if (found) return found->n;
    return NULL;
}

void ajouter_arete(Noeud *parent, Noeud *child, double fuite_pct) {
    Enfant *e;

    if (!parent || !child) return;

    e = malloc(sizeof(Enfant));
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

/* -------- HISTO -------- */

const char* entete_histo(ModeHisto mode) {
    if (mode == HISTO_MAX)  return "identifier;max volume (M.m3.year-1)\n";
    if (mode == HISTO_SRC)  return "identifier;source volume (M.m3.year-1)\n";
    if (mode == HISTO_REAL) return "identifier;real volume (M.m3.year-1)\n";
    return "identifier;real;lost(src-real);remaining(max-src) (M.m3.year-1)\n";
}

/* Traite le mode histo :
   - Lecture du fichier CSV
   - Construction d'un AVL d'usines (clé = id)
   - Écriture du fichier .dat dans output/
*/
int traiter_histo(const char *chemin_fichier, ModeHisto mode, const char *out_dat) {
    FILE *in;
    FILE *out;
    AVLUsine *arbre;
    int h;
        int err;
char ligne[MAX_LIGNE];
    in = fopen(chemin_fichier, "r");
    if (!in) {
        fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", chemin_fichier);
        return 10;
    }

    arbre = NULL;
    h = 0;

    err = 0;
    while (lire_ligne(in, ligne, MAX_LIGNE)) {
        char *col[5];

        if (!split_5_colonnes(ligne, col)) continue;

        /* volume en col[3] pour source->usine et pour usine node ; si '-' => ignorer */
        if (est_tiret_ou_vide(col[3])) {
            continue;
        }

        /* USINE (node) : col[2] = "-" et col[1] = ID usine */
        if (est_tiret_ou_vide(col[2]) && !est_tiret_ou_vide(col[1])) {
            const char *id_usine;
            int ok;
            double cap_km3;
            AVLUsine *node;

            id_usine = col[1];

            ok = 0;
            cap_km3 = atof_safe(col[3], &ok);
            if (!ok) continue;

            node = avlU_rechercher(arbre, id_usine);
            if (!node) {
                UsineDonnees *u = malloc(sizeof(UsineDonnees));
                if (!u) {
                    fprintf(stderr, "Erreur: mémoire insuffisante (usine).\n");
                    fclose(in);
                    avlU_liberer(arbre);
                    return 11;
                }

                memset(u, 0, sizeof(UsineDonnees));
                strncpy(u->id_usine, id_usine, MAX_ID - 1);
                u->id_usine[MAX_ID - 1] = '\0';
                u->capacite_max_km3 = cap_km3;

                err = 0;


                arbre = avlU_inserer(arbre, u, &h, &err);
                if (err) {
                    fprintf(stderr, "Erreur: mémoire insuffisante (construction AVL usines).\n");
                    fclose(in);
                    avlU_liberer(arbre);
                    return 14;
                }
            } else {
                node->u->capacite_max_km3 = cap_km3;
            }
            continue;
        }

        /* SOURCE -> USINE : col[2] = usine ; col[3] = volume ; col[4] = fuite */
        if (!est_tiret_ou_vide(col[2]) && !est_tiret_ou_vide(col[3])) {
            const char *id_usine;
            int okV;
            double vol;
            double fuite;
            int okF;
            AVLUsine *node;

            id_usine = col[2];

            okV = 0;
            vol = atof_safe(col[3], &okV);
            if (!okV) continue;

            fuite = 0.0;
            if (!est_tiret_ou_vide(col[4])) {
                okF = 0;
                fuite = atof_safe(col[4], &okF);
                if (!okF) fuite = 0.0;
            }

            node = avlU_rechercher(arbre, id_usine);
            if (!node) {
                UsineDonnees *u = malloc(sizeof(UsineDonnees));
                if (!u) {
                    fprintf(stderr, "Erreur: mémoire insuffisante (source->usine).\n");
                    fclose(in);
                    avlU_liberer(arbre);
                    return 12;
                }

                memset(u, 0, sizeof(UsineDonnees));
                strncpy(u->id_usine, id_usine, MAX_ID - 1);
                u->id_usine[MAX_ID - 1] = '\0';

                err = 0;


                arbre = avlU_inserer(arbre, u, &h, &err);
                if (err) {
                    fprintf(stderr, "Erreur: mémoire insuffisante (construction AVL usines).\n");
                    fclose(in);
                    avlU_liberer(arbre);
                    return 14;
                }
                node = avlU_rechercher(arbre, id_usine);
                if (!node) {
                    fprintf(stderr, "Erreur interne: insertion AVL usine échouée.\n");
                    fclose(in);
                    avlU_liberer(arbre);
                    return 13;
                }
            }

            node->u->volume_src_km3 += vol;
            node->u->volume_real_km3 += vol * (1.0 - (fuite / 100.0));
        }
    }

    fclose(in);

    out = fopen(out_dat, "w");
    if (!out) {
        fprintf(stderr, "Erreur: impossible de créer %s (crée output/)\n", out_dat);
        avlU_liberer(arbre);
        return 20;
    }

    fputs(entete_histo(mode), out);
    avlU_ecrire_inverse(arbre, out, mode);
    fclose(out);

    if (mode != HISTO_MAX) {
        FILE *out_max = fopen("output/histo_max.dat", "w");
        if (out_max) {
            fputs(entete_histo(HISTO_MAX), out_max);
            avlU_ecrire_inverse(arbre, out_max, HISTO_MAX);
            fclose(out_max);
        }
    }

    avlU_liberer(arbre);
    return 0;
}

/* -------- LEAKS -------- */

int fichier_vide_ou_absent(const char *path) {
    FILE *f;
    long sz;

    f = fopen(path, "r");
    if (!f) return 1;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 1;
    }
    sz = ftell(f);
    fclose(f);

    if (sz <= 0) return 1;
    return 0;
}

/* Construit le chemin du fichier bonus (dans le même dossier que out_dat). */
void construire_chemin_leaks_bonus(const char *out_dat, char *out_bonus, int taille) {
    const char *slash;
    int dirlen;

    if (!out_bonus || taille <= 0) return;

    /* Par défaut (si out_dat n'a pas de dossier) */
    strncpy(out_bonus, "leaks_max_segment.dat", (size_t)taille - 1);
    out_bonus[taille - 1] = '\0';

    if (!out_dat) return;

    slash = strrchr(out_dat, '/');
    if (!slash) return;

    dirlen = (int)(slash - out_dat) + 1; /* inclut le '/' */
    if (dirlen <= 0) return;
    if (dirlen >= taille) dirlen = taille - 1;

    memcpy(out_bonus, out_dat, (size_t)dirlen);
    out_bonus[dirlen] = '\0';

    strncat(out_bonus, "leaks_max_segment.dat", (size_t)taille - (size_t)dirlen - 1);
}

/* Traite la commande leaks :
   - Vérifie que l'usine existe
   - Calcule le débit "real" arrivant à l'usine
   - Construit le graphe aval (filtré sur l'usine)
   - Propage le débit et additionne les pertes
*/
int traiter_leaks(const char *chemin_fichier, const char *id_usine, const char *out_dat) {
    FILE *in;
    FILE *out;
    int need_header;


    /* -------- Bonus : tronçon avec pertes max (volume absolu) -------- */
    char out_bonus[512];
    int need_header_bonus;

    int usine_trouvee;
    double real_km3;
    /* -------- Phase 1 : existence usine + débit "real" (km3) -------- */
    in = fopen(chemin_fichier, "r");
    if (!in) {
        fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", chemin_fichier);
        return 30;
    }

    usine_trouvee = 0;
    real_km3 = 0.0;

    {
        char ligne[MAX_LIGNE];
        while (lire_ligne(in, ligne, MAX_LIGNE)) {
            char *col[5];

            if (!split_5_colonnes(ligne, col)) continue;

            /* ligne USINE (node) : col[1] = usine, col[2] = "-" */
            if (est_tiret_ou_vide(col[2]) && !est_tiret_ou_vide(col[1])) {
                if (strcmp(col[1], id_usine) == 0) {
                    usine_trouvee = 1;
                }
                continue;
            }

            /* source->usine : col[2] = usine, col[3]=vol, col[4]=fuite */
            if (!est_tiret_ou_vide(col[2]) && !est_tiret_ou_vide(col[3])) {
                if (strcmp(col[2], id_usine) == 0) {
                    int okV;
                    double vol;
                    double fuite;
                    int okF;

                    okV = 0;
                    vol = atof_safe(col[3], &okV);
                    if (!okV) continue;

                    fuite = 0.0;
                    if (!est_tiret_ou_vide(col[4])) {
                        okF = 0;
                        fuite = atof_safe(col[4], &okF);
                        if (!okF) fuite = 0.0;
                    }
                    real_km3 += vol * (1.0 - (fuite / 100.0));
                }
            }
        }
    }

    fclose(in);

    need_header = fichier_vide_ou_absent(out_dat);


    construire_chemin_leaks_bonus(out_dat, out_bonus, (int)sizeof(out_bonus));
    need_header_bonus = fichier_vide_ou_absent(out_bonus);

    /* usine inconnue => écrire -1 */
    if (!usine_trouvee) {
        out = fopen(out_dat, "a");
        if (!out) {
            fprintf(stderr, "Erreur: impossible d'ouvrir %s en écriture\n", out_dat);
            return 31;
        }
        if (need_header) fputs("identifier;Leak volume (M.m3.year-1)\n", out);
        fprintf(out, "%s;-1.000000\n", id_usine);
        fclose(out);


        /* fichier bonus : mêmes règles (-1 si usine inconnue) */
        {
            FILE *outB = fopen(out_bonus, "a");
            if (outB) {
                if (need_header_bonus) fputs("identifier;Upstream;Downstream;Max leak volume (M.m3.year-1)\n", outB);
                fprintf(outB, "%s;-;-;-1.000000\n", id_usine);
                fclose(outB);
            }
        }
        return 0;
    }

    /* -------- Phase 2 : construire le graphe aval pour cette usine -------- */
    {
        AVLNoeud *index;
        Noeud *racine;

        in = fopen(chemin_fichier, "r");
        if (!in) {
            fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", chemin_fichier);
            return 33;
        }

        index = NULL;

        racine = obtenir_ou_creer_noeud(&index, id_usine);
        if (!racine) {
            fprintf(stderr, "Erreur: mémoire insuffisante (création racine leaks).\n");
            fclose(in);
            liberer_graphe(index);
            return 32;
        }

        {
            char ligne[MAX_LIGNE];

            while (lire_ligne(in, ligne, MAX_LIGNE)) {
                char *col[5];
                const char *usine_traitante;
                const char *amont;
                const char *aval;

                int okF;
                double fuite;

                int est_usine_stockage;
                int est_aval_filtre;

                Noeud *p;
                Noeud *c;

                if (!split_5_colonnes(ligne, col)) continue;

                usine_traitante = col[0];
                amont = col[1];
                aval  = col[2];

                /* lien direct usine -> stockage : amont=usine, aval!=-, fuite en col[4] */
                est_usine_stockage = (strcmp(amont, id_usine) == 0)
                                     && !est_tiret_ou_vide(aval)
                                     && !est_tiret_ou_vide(col[4]);

                /* tronçons aval : col[0] = usine traitante */
                est_aval_filtre = (strcmp(usine_traitante, id_usine) == 0)
                                  && !est_tiret_ou_vide(amont)
                                  && !est_tiret_ou_vide(aval)
                                  && !est_tiret_ou_vide(col[4]);

                if (!est_usine_stockage && !est_aval_filtre) continue;

                okF = 0;
                fuite = atof_safe(col[4], &okF);
                if (!okF) fuite = 0.0;

                p = obtenir_ou_creer_noeud(&index, amont);
                c = obtenir_ou_creer_noeud(&index, aval);
                if (!p || !c) {
                    fprintf(stderr, "Erreur: mémoire insuffisante lors de la construction du graphe leaks.\n");
                    fclose(in);
                    liberer_graphe(index);
                    return 34;
                }

                ajouter_arete(p, c, fuite);
            }
        }

        fclose(in);

        /* -------- Phase 3 : propagation et calcul des pertes --------
           - Le débit qui arrive à un noeud est réparti équitablement entre ses sorties (deg).
           - Sur chaque tronçon, on perd une part (fuite_pct%).
           - Le reste continue sa propagation.
        */
        {
            double debit0;
            double pertes_Mm3;


            /* Bonus : suivi du tronçon qui perd le plus (en volume absolu) */
            double max_perdu;
            char max_amont[MAX_ID];
            char max_aval[MAX_ID];

            ElementPile *pile;
            int cap;
            int top;

            debit0 = real_km3 / 1000.0; /* km3 -> M.m3 */
            pertes_Mm3 = 0.0;


            max_perdu = -1.0;
            strcpy(max_amont, "-");
            strcpy(max_aval, "-");

            cap = 1024;
            top = 0;
            pile = malloc(cap * sizeof(ElementPile));
            if (!pile) {
                fprintf(stderr, "Erreur: mémoire insuffisante (pile propagation leaks).\n");
                liberer_graphe(index);
                return 35;
            }

            pile[top].n = racine;
            pile[top].debit = debit0;
            top++;

            while (top > 0) {
                ElementPile it;
                Noeud *n;
                double debit;
                double part;
                Enfant *e;

                top--;
                it = pile[top];
                n = it.n;
                debit = it.debit;

                if (!n || n->deg <= 0) continue;

                part = debit / (double)n->deg;

                e = n->enfants;
                while (e) {
                    double f;
                    double perdu;
                    double restant;

                    f = e->fuite_pct / 100.0;
                    if (f < 0.0) f = 0.0;
                    if (f > 1.0) f = 1.0;

                    perdu = part * f;
                    restant = part - perdu;
                    pertes_Mm3 += perdu;


                    /* Bonus : conserver le max */
                    if (perdu > max_perdu) {
                        max_perdu = perdu;

                        strncpy(max_amont, n->id, MAX_ID - 1);
                        max_amont[MAX_ID - 1] = '\0';

                        if (e->child) {
                            strncpy(max_aval, e->child->id, MAX_ID - 1);
                            max_aval[MAX_ID - 1] = '\0';
                        } else {
                            strcpy(max_aval, "-");
                        }
                    }



                    if (restant > 0.0 && e->child && e->child->deg > 0) {
                        if (top >= cap) {
                            ElementPile *tmp;
                            cap *= 2;
                            tmp = realloc(pile, cap * sizeof(ElementPile));
                            if (!tmp) {
                                fprintf(stderr, "Erreur: mémoire insuffisante (realloc pile leaks).\n");
                                free(pile);
                                liberer_graphe(index);
                                return 36;
                            }
                            pile = tmp;
                        }
                        pile[top].n = e->child;
                        pile[top].debit = restant;
                        top++;
                    }

                    e = e->next;
                }
            }

            free(pile);
            liberer_graphe(index);

            out = fopen(out_dat, "a");
            if (!out) {
                fprintf(stderr, "Erreur: impossible d'ouvrir %s en écriture\n", out_dat);
                return 40;
            }
            if (need_header) fputs("identifier;Leak volume (M.m3.year-1)\n", out);
            fprintf(out, "%s;%.6f\n", id_usine, pertes_Mm3);
            fclose(out);


            /* fichier bonus : tronçon qui perd le plus */
            {
                FILE *outB = fopen(out_bonus, "a");
                if (outB) {
                    if (need_header_bonus) fputs("identifier;Upstream;Downstream;Max leak volume (M.m3.year-1)\n", outB);

                    /* si aucun tronçon (deg=0 partout), max_perdu peut rester à -1 */
                    if (max_perdu < 0.0) {
                        fprintf(outB, "%s;-;-;0.000000\n", id_usine);
                    } else {
                        fprintf(outB, "%s;%s;%s;%.6f\n", id_usine, max_amont, max_aval, max_perdu);
                    }
                    fclose(outB);
                }
            }
        }
    }

    return 0;
}
