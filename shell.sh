#!/bin/bash

# Durée totale du script (ms)
START_TOTAL=$(date +%s%N)
finish() {
  END_TOTAL=$(date +%s%N)
  DURATION=$(( (END_TOTAL - START_TOTAL) / 1000000 ))
  echo "Durée totale du script : ${DURATION} ms"
}
trap finish EXIT

usage() {
  echo "Usage:"
  echo "  $0 <datafile> histo <max|src|real>"
  echo "  $0 <datafile> leaks \"<ID usine>\""
}

# On attend 3 arguments. (Pour leaks : mettre l'ID entre guillemets si besoin.)
if [ $# -ne 3 ]; then
  echo "Erreur: nombre d'arguments invalide."
  usage
  exit 1
fi

DATAFILE="$1"
CMD="$2"
OPT="$3"

if [ ! -f "$DATAFILE" ]; then
  echo "Erreur: fichier introuvable : $DATAFILE"
  exit 1
fi

# Compile si besoin
if [ ! -x "./projet" ]; then
  echo "Compilation (make)..."
  make
  if [ $? -ne 0 ]; then
    echo "Erreur: compilation échouée."
    exit 1
  fi
fi

mkdir -p tmp

# Exécution C
./projet "$DATAFILE" "$CMD" "$OPT"
RET=$?
if [ $RET -ne 0 ]; then
  echo "Erreur: le programme C a retourné $RET"
  exit $RET
fi

# Gnuplot uniquement pour histo
if [ "$CMD" = "histo" ]; then
  case "$OPT" in
    max) DAT="histo_max.dat" ;;
    src) DAT="histo_src.dat" ;;
    real) DAT="histo_real.dat" ;;
    *) echo "Erreur: option histo invalide"; exit 1 ;;
  esac

  if [ ! -f "$DAT" ]; then
    echo "Erreur: fichier de sortie manquant: $DAT"
    exit 1
  fi

  # Préparer top10 / bot50 (sans l'en-tête)
  tail -n +2 "$DAT" | sort -t";" -k2 -nr | head -n 10 > tmp/top10.dat
  tail -n +2 "$DAT" | awk -F";" '$2 > 0' | sort -t";" -k2 -n | head -n 50 > tmp/bot50.dat

  # Générer PNG (noms = noms du .dat + suffixes)
  HIGH="${DAT%.dat}_high.png"
  LOW="${DAT%.dat}_low.png"

  # On envoie directement des commandes à gnuplot (here-document)
  gnuplot <<EOF
set terminal pngcairo size 1400,800 enhanced font 'Verdana,10'
set output '${HIGH}'
set datafile separator ";"
set title "Plant histogram (${OPT}) - 10 greatest" font ",18"
set style data histograms
set style fill solid 1.0 border -1
set boxwidth 0.85
set ylabel "Volume (M.m3.year-1)"
set xtics rotate by -90
plot 'tmp/top10.dat' using 2:xtic(1) title '${OPT}'
EOF

  gnuplot <<EOF
set terminal pngcairo size 1400,800 enhanced font 'Verdana,9'
set output '${LOW}'
set datafile separator ";"
set title "Plant histogram (${OPT}) - 50 lowest" font ",18"
set style data histograms
set style fill solid 1.0 border -1
set boxwidth 0.85
set ylabel "Volume (M.m3.year-1)"
set xtics rotate by -90
plot 'tmp/bot50.dat' using 2:xtic(1) title '${OPT}'
EOF

  echo "OK: ${DAT} -> ${HIGH} / ${LOW}"
fi

if [ "$CMD" = "leaks" ]; then
  echo "OK: leaks.dat mis à jour"
fi
