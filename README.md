# Projet WildWater P2-MEF2-ANT


## Projet réalisé en groupe par :
- Ahmed EISH
- Naim MOHAMMED
- Tarek ITIM  

**Année :** Préing 2 - MEF2 - 2025-26

---

## Description

WildWater est un projet en **langage C**, accompagné d’un script **Shell**, permettant d’analyser un fichier de données volumineux représentant un réseau de distribution d’eau.

Le programme réalise différents traitements sur les données afin de :
- générer des **histogrammes** selon plusieurs critères (sources, valeurs maximales, valeurs réelles),
- identifier des **fuites (leaks)** associées à une usine donnée,
- produire des fichiers de sortie exploitables et des visualisations graphiques.

Un mode supplémentaire (*bonus*) permet de générer un histogramme cumulant plusieurs valeurs.

---

## Prérequis

- Système **Linux / Unix**
- `gcc` et `make` pour la compilation
- `gnuplot` pour la génération des graphiques
- Un fichier de données au format `.dat` ou `.csv`

---

## Compilation

Depuis la racine du projet, exécuter successivement les commandes suivantes :

```bash
make clean
make
chmod +x shell.sh
