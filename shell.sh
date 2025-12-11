#!/bin/bash

# Aide
if [ "$1" = "-h" ]; then
    echo "Usage: $0 [fichier.csv] [commande] [option]"
    echo "Commandes:"
    echo "  histo : Génère les graphiques des usines"
    echo "  leaks : (Non implémenté) Analyse des fuites"
    exit 0
fi

# Verification arguments
if [ $# -lt 2 ]; then
    echo "Erreur: arguments manquants."
    exit 1
fi

INPUT_FILE=$1
CMD=$2

# Verification existence fichier
if [ ! -f "$INPUT_FILE" ]; then
    echo "Erreur: Fichier $INPUT_FILE introuvable."
    exit 1
fi

# Compilation
if [ ! -f "projet" ]; then
    echo "Compilation en cours..."
    make
    if [ $? -ne 0 ]; then
        echo "Erreur de compilation."
        exit 1
    fi
fi

# Nettoyage
mkdir -p tmp
rm -f tmp/*.dat tmp/*.csv

# Execution et mesure du temps
START=$(date +%s%N) # Nanosecondes
./projet "$INPUT_FILE" "$CMD"
RET=$?
END=$(date +%s%N)

DURATION=$(( ($END - $START) / 1000000 ))
echo "Durée du traitement C : ${DURATION} ms"

if [ $RET -ne 0 ]; then
    echo "Le programme C a retourné une erreur."
    exit 1
fi

# Gestion des graphiques pour HISTO
if [ "$CMD" = "histo" ]; then
    if [ -f "data_usine.csv" ]; then
        echo "Génération des graphiques..."

        # Tri décroissant sur la capacité (colonne 2) pour les 10 plus grands
        sort -t";" -k2 -nr data_usine.csv | head -n 10 > tmp/top10.dat
        
        # Tri croissant sur la capacité pour les 50 plus petits
        # On filtre ceux qui ont une capacité > 0 pour éviter les erreurs
        awk -F";" '$2 > 0' data_usine.csv | sort -t";" -k2 -n | head -n 50 > tmp/bot50.dat

        # Gnuplot pour le BONUS (Stacked Histogram) - Top 10
        # On veut superposer : Source (vert), Traité (bleu), Capacité (rouge/fond) ?
        # Selon l'image fournie : 
        # Large barre rouge fond = Capacité
        # Barre verte = Source (Capté)
        # Barre bleue (dans le vert) = Traité (Réel)
        
        gnuplot << EOF
        set terminal pngcairo size 1200,800 enhanced font 'Verdana,10'
        set output 'histo_all_high.png'
        set title "Plant data (10 greatest)" font ",20"
        set style data histograms
        set style histogram cluster gap 1
        set style fill solid 1.0 border -1
        set datafile separator ";"
        set ylabel "Volume (M.m3)" font ",16"
        set xtics rotate by -90
        set boxwidth 0.9 absolute
        
        # Astuce pour l'effet "imbriqué" (Bonus):
        # On dessine la capacité (large), puis le captage (moins large), puis le traité
        
        plot 'tmp/top10.dat' using 2:xtic(1) title 'Capacity' lc rgb "#ffaaaa", \
             ''              using 3 title 'Source' lc rgb "#ccffcc", \
             ''              using 4 title 'Treated' lc rgb "#ccccff"
EOF

        # Gnuplot pour les 50 plus petits
        gnuplot << EOF
        set terminal pngcairo size 1200,800 enhanced font 'Verdana,8'
        set output 'histo_all_low.png'
        set title "Plant data (50 lowest)" font ",20"
        set style data histograms
        set style fill solid 1.0 border -1
        set datafile separator ";"
        set ylabel "Volume (M.m3)" font ",16"
        set xtics rotate by -90
        
        plot 'tmp/bot50.dat' using 2:xtic(1) title 'Capacity' lc rgb "#ffaaaa", \
             ''              using 3 title 'Source' lc rgb "#ccffcc", \
             ''              using 4 title 'Treated' lc rgb "#ccccff"
EOF
        
        echo "Images générées : histo_all_high.png et histo_all_low.png"
    else
        echo "Erreur: Pas de données en sortie."
    fi
fi