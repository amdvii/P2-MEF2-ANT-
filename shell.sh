#!/bin/bash

usage() {
  echo "Usage:"
  echo "  $0 <datafile> histo <max|src|real|all>"
  echo "  $0 <datafile> leaks \"<ID usine>\""
}

# Chrono (ms) 
now_ms() {
  if date +%s%N >/dev/null 2>&1; then
    echo $(( $(date +%s%N) / 1000000 ))
  else
    echo $(( $(date +%s) * 1000 ))
  fi
}

START_TOTAL=$(now_ms)
finish() {
  END_TOTAL=$(now_ms)
  echo "Durée totale du script : $((END_TOTAL - START_TOTAL)) ms"
}
trap finish EXIT

if [ $# -ne 3 ]; then
  echo "Erreur: mauvais nombre d'arguments."
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

# Compilation si besoin
if [ ! -x "./projet" ]; then
  make || exit 1
fi

OUTDIR="output"
TMPDIR="tmp"
mkdir -p "$OUTDIR" "$TMPDIR"

./projet "$DATAFILE" "$CMD" "$OPT"
RET=$?
if [ $RET -ne 0 ]; then
  echo "Erreur: le programme C a retourné $RET"
  exit $RET
fi

if [ "$CMD" = "histo" ]; then
  case "$OPT" in
    max)  DAT="$OUTDIR/histo_max.dat" ;;
    src)  DAT="$OUTDIR/histo_src.dat" ;;
    real) DAT="$OUTDIR/histo_real.dat" ;;
    all)  DAT="$OUTDIR/histo_all.dat" ;;
    *) echo "Erreur: option histo invalide"; usage; exit 1 ;;
  esac

  if ! command -v gnuplot >/dev/null 2>&1; then
    echo "Erreur: gnuplot n'est pas installé (ou pas dans le PATH)."
    exit 1
  fi

  # Sélection top/bot
  REF="$OUTDIR/histo_max.dat"
  if [ ! -f "$REF" ]; then
    ./projet "$DATAFILE" histo max || exit 1
  fi

  tail -n +2 "$REF" | sort -t";" -k2 -nr | head -n 10 | cut -d";" -f1 > "$TMPDIR/top10_ids.txt"
  tail -n +2 "$REF" | awk -F";" '$2 > 0' | sort -t";" -k2 -n | head -n 50 | cut -d";" -f1 > "$TMPDIR/bot50_ids.txt"

  if [ "$OPT" = "all" ]; then
    # id;real;lost;remaining  (bleu;rouge;vert)
    awk -F';' 'NR==FNR{ids[++n]=$1; next}
      FNR>1{c2[$1]=$2; c3[$1]=$3; c4[$1]=$4}
      END{for(i=1;i<=n;i++){id=ids[i]; printf "%s;%s;%s;%s\n", id,
            (id in c2?c2[id]:0), (id in c3?c3[id]:0), (id in c4?c4[id]:0)}}' \
      "$TMPDIR/top10_ids.txt" "$DAT" > "$TMPDIR/top10.dat"

    awk -F';' 'NR==FNR{ids[++n]=$1; next}
      FNR>1{c2[$1]=$2; c3[$1]=$3; c4[$1]=$4}
      END{for(i=1;i<=n;i++){id=ids[i]; printf "%s;%s;%s;%s\n", id,
            (id in c2?c2[id]:0), (id in c3?c3[id]:0), (id in c4?c4[id]:0)}}' \
      "$TMPDIR/bot50_ids.txt" "$DAT" > "$TMPDIR/bot50.dat"
  else
    # id;val
    awk -F';' 'NR==FNR{ids[++n]=$1; next}
      FNR>1{val[$1]=$2}
      END{for(i=1;i<=n;i++){id=ids[i]; printf "%s;%s\n", id, (id in val?val[id]:0)}}' \
      "$TMPDIR/top10_ids.txt" "$DAT" > "$TMPDIR/top10.dat"

    awk -F';' 'NR==FNR{ids[++n]=$1; next}
      FNR>1{val[$1]=$2}
      END{for(i=1;i<=n;i++){id=ids[i]; printf "%s;%s\n", id, (id in val?val[id]:0)}}' \
      "$TMPDIR/bot50_ids.txt" "$DAT" > "$TMPDIR/bot50.dat"
  fi

  BASE="$(basename "$DAT" .dat)"
  HIGH="$OUTDIR/${BASE}_high.png"
  LOW="$OUTDIR/${BASE}_low.png"

  if [ "$OPT" = "all" ]; then
    gnuplot <<EOF
set terminal pngcairo size 1500,850 enhanced font 'Verdana,10'
set output '${HIGH}'
set datafile separator ";"
set title "Plant histogram (all) - 10 greatest" font ",18"
set style data histograms
set style histogram rowstacked
set style fill solid 1.0 border -1
set boxwidth 0.85
set ylabel "Volume (M.m3.year-1)"
set xtics rotate by -90
plot '${TMPDIR}/top10.dat' using 2:xtic(1) title 'real' lc rgb 'blue', \
     '' using 3 title 'src-real' lc rgb 'red', \
     '' using 4 title 'max-src' lc rgb 'green'
EOF

    gnuplot <<EOF
set terminal pngcairo size 1500,850 enhanced font 'Verdana,9'
set output '${LOW}'
set datafile separator ";"
set title "Plant histogram (all) - 50 lowest" font ",18"
set style data histograms
set style histogram rowstacked
set style fill solid 1.0 border -1
set boxwidth 0.85
set ylabel "Volume (M.m3.year-1)"
set xtics rotate by -90
plot '${TMPDIR}/bot50.dat' using 2:xtic(1) title 'real (bleu)' lc rgb 'blue', \
     '' using 3 title 'src-real (rouge)' lc rgb 'red', \
     '' using 4 title 'max-src (vert)' lc rgb 'green'
EOF
  else
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
plot '${TMPDIR}/top10.dat' using 2:xtic(1) title '${OPT}'
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
plot '${TMPDIR}/bot50.dat' using 2:xtic(1) title '${OPT}'
EOF
  fi

  echo "OK: ${DAT} -> ${HIGH} / ${LOW}"
fi

if [ "$CMD" = "leaks" ]; then
  echo "OK: $OUTDIR/leaks.dat mis à jour"
fi
