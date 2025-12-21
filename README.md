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
```

---

## Utilisation

Le script `shell.sh` constitue le point d’entrée principal du projet.

### Génération des histogrammes

- Histogramme des valeurs **sources** :
```bash
./shell.sh c-wildwater_v3.dat histo src
```
Histogramme des valeurs **maximales** :
```bash
./shell.sh c-wildwater_v3.dat histo max
```
Histogramme des valeurs **réelles** :
```bash
./shell.sh c-wildwater_v3.dat histo real
```
Histogramme **cumulé** (bonus) :
```bash
./shell.sh c-wildwater_v3.dat histo all
```

### Recherche des fuites (leaks)

Permet d’obtenir les informations de fuite associées à une usine précise.

```bash
./shell.sh c-wildwater_v3.dat leaks "<ID usine>"
```
Exemple d’utilisation :
```bash 
shell.sh c-wildwater_v3.dat leaks "Unit #NM000000T"
```
