#!/bin/bash

# Fonction d'aide
if [ "$1" = "-h" ]; then
    echo "Usage: $0 [fichier_dat] [type_station] [type_conso] [id_station]"
    echo "  type_station : hvb, hva, lv"
    echo "  type_conso   : comp, indiv, all"
    echo "  id_station   : Optionnel (pour le mode leaks)"
    exit 0
fi

# Verification du nombre d'arguments
if [ $# -lt 3 ]; then
    echo "Erreur: arguments manquants. Utilisez -h pour l'aide."
    exit 1
fi

FICHIER=$1
STATION=$2
CONSO=$3
ID=$4

# Verification de l'existence du fichier
if [ ! -f "$FICHIER" ]; then
    echo "Erreur: le fichier $FICHIER n'existe pas."
    exit 1
fi

# Verification des types de stations
if [ "$STATION" != "hvb" ] && [ "$STATION" != "hva" ] && [ "$STATION" != "lv" ]; then
    echo "Erreur: type de station invalide ($STATION)."
    exit 1
fi

# Verification des types de consommateurs
if [ "$CONSO" != "comp" ] && [ "$CONSO" != "indiv" ] && [ "$CONSO" != "all" ]; then
    echo "Erreur: type de consommateur invalide ($CONSO)."
    exit 1
fi

# Verification des combinaisons interdites
if [ "$STATION" = "hvb" ] || [ "$STATION" = "hva" ]; then
    if [ "$CONSO" = "all" ] || [ "$CONSO" = "indiv" ]; then
        echo "Erreur: combinaison station/consommateur interdite."
        exit 1
    fi
fi

# Creation des dossiers temporaires et graphiques
if [ ! -d "tmp" ]; then
    mkdir tmp
fi
if [ ! -d "graphs" ]; then
    mkdir graphs
fi

# Nettoyage du dossier tmp
rm -f tmp/*

# Compilation du projet
echo "Compilation..."
make
if [ $? -ne 0 ]; then
    echo "Erreur lors de la compilation."
    exit 1
fi

# Execution du programme C
echo "Traitement des donnees..."
DEBUT=$(date +%s)

# Lancement de l'executable avec les parametres
./projet "$FICHIER" "$STATION" "$CONSO" "$ID"

if [ $? -ne 0 ]; then
    echo "Erreur lors de l'execution du programme C."
    exit 1
fi

FIN=$(date +%s)
DUREE=$((FIN - DEBUT))
echo "Duree du traitement : $DUREE s"

# Traitement des graphiques (si ce n'est pas le mode leaks)
if [ -z "$ID" ]; then
    echo "Generation du graphique..."
    
    # Le fichier de sortie du C est suppose etre tmp/output.csv
    # On trie les donnees par la 2eme colonne (numerique decroissant)
    # On garde les 10 premiers (min et max a adapter selon le besoin)
    sort -t ";" -k 2 -n -r tmp/data.csv | head -n 10 > tmp/data_sorted.dat

    # Generation de l'image avec Gnuplot
    gnuplot << EOF
    set terminal png size 800,600
    set output 'graphs/resultat.png'
    set title "Graphique $STATION $CONSO"
    set style data histograms
    set style fill solid
    set datafile separator ";"
    set ylabel "KWh / m3"
    set xlabel "ID Station"
    set xtics rotate
    plot "tmp/data_sorted.dat" using 2:xtic(1) title "Volume"
EOF

    echo "Graphique disponible dans graphs/resultat.png"
else
    # Mode leaks : affichage simple
    if [ -f "tmp/data.csv" ]; then
        echo "Resultat :"
        cat tmp/data.csv
    fi
fi