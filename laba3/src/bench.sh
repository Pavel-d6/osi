#!/bin/bash
export LC_ALL=C

EXE="./main.o"
SIZES="500 1000 2000"
THREADS="1 2 4 8 16 32"
REPEATS=3

run() {
  for ((r = 0; r < REPEATS; r++)); do
    "$EXE" -t "$1" -n "$2" | head -n 1 | awk '{print $3}'
  done | sort -n | sed -n "$(( (REPEATS + 1) / 2 ))p"
}

for n in $SIZES; do
  echo "N=$n"
  printf "%-8s %-12s %-8s %-8s\n" "потоки" " время, с" "     S" "     E"
  t1=""
  for p in $THREADS; do
    t=$(run "$p" "$n")
    [ -z "$t1" ] && t1=$t
    awk -v p="$p" -v t="$t" -v t1="$t1" \
      'BEGIN { printf "%-8d %-12.4f %-8.2f %-8.2f\n", p, t, t1/t, t1/t/p }'
  done
  echo
done